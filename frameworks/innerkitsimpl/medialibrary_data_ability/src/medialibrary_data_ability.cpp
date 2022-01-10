/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "medialibrary_data_ability.h"

using namespace std;
using namespace OHOS::AppExecFwk;
using namespace OHOS::NativeRdb;

namespace OHOS {
namespace Media {
REGISTER_AA(MediaLibraryDataAbility);

void MediaLibraryDataAbility::OnStart(const AAFwk::Want &want)
{
    MEDIA_INFO_LOG("MediaLibraryDataAbility::OnStart");
    Ability::OnStart(want);
    InitMediaLibraryRdbStore();
}

void MediaLibraryDataAbility::OnStop()
{
    MEDIA_INFO_LOG("MediaLibraryDataAbility::OnStop");
    Ability::OnStop();
    rdbStore = nullptr;
    isRdbStoreInitialized = false;

    if (scannerClient_ != nullptr) {
        scannerClient_->Release();
        scannerClient_ = nullptr;
    }
}

MediaLibraryDataAbility::MediaLibraryDataAbility(void)
{
    isRdbStoreInitialized = false;
    rdbStore = nullptr;
}

MediaLibraryDataAbility::~MediaLibraryDataAbility(void) {}

int32_t MediaLibraryDataCallBack::OnCreate(RdbStore &store)
{
    int32_t error_code = NativeRdb::E_ERROR;
    error_code = store.ExecuteSql(CREATE_MEDIA_TABLE);
    if (error_code == NativeRdb::E_OK) {
        error_code = store.ExecuteSql(CREATE_SMARTALBUM_TABLE);
    }
    if (error_code == NativeRdb::E_OK) {
        error_code = store.ExecuteSql(CREATE_SMARTALBUMMAP_TABLE);
    }
    if (error_code == NativeRdb::E_OK) {
        error_code = store.ExecuteSql(CREATE_DEVICE_TABLE);
    }
    if (error_code == NativeRdb::E_OK) {
        error_code = store.ExecuteSql(CREATE_IMAGE_VIEW);
    }
    if (error_code == NativeRdb::E_OK) {
        error_code = store.ExecuteSql(CREATE_VIDEO_VIEW);
    }
    if (error_code == NativeRdb::E_OK) {
        error_code = store.ExecuteSql(CREATE_AUDIO_VIEW);
    }
    if (error_code == NativeRdb::E_OK) {
        error_code= store.ExecuteSql(CREATE_ABLUM_VIEW);
    }
    return error_code;
}

int32_t MediaLibraryDataCallBack::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return E_OK;
}

int32_t MediaLibraryDataAbility::InitMediaLibraryRdbStore()
{
    if (isRdbStoreInitialized) {
        return DATA_ABILITY_SUCCESS;
    }
    int32_t errCode(DATA_ABILITY_FAIL);
    RdbStoreConfig config(MEDIA_DATA_ABILITY_DB_NAME);
    MediaLibraryDataCallBack rdbDataCallBack;
    rdbStore = RdbHelper::GetRdbStore(config, MEDIA_RDB_VERSION, rdbDataCallBack, errCode);
    if (rdbStore == nullptr) {
        MEDIA_ERR_LOG("MediaLibraryDataAbility::InitMediaRdbStore GetRdbStore is failed ");
        return errCode;
    }
    isRdbStoreInitialized = true;
    MEDIA_INFO_LOG("DATA_ABILITY_SUCCESS");
    return DATA_ABILITY_SUCCESS;
}
int32_t MediaLibraryDataAbility::Insert(const Uri &uri, const ValuesBucket &value)
{
    MEDIA_INFO_LOG("MediaLibraryDataAbility Insert: IN");
    if ((!isRdbStoreInitialized) || (value.IsEmpty()) || (rdbStore == nullptr)) {
        MEDIA_ERR_LOG("MediaLibraryDataAbility Insert: Input parameter is invalid");
        return DATA_ABILITY_FAIL;
    }
    
    string insertUri = uri.ToString();
    // If insert uri contains media opearation, follow media operation procedure
    if (insertUri.find(MEDIA_OPERN_KEYWORD) != string::npos) {
        MediaLibraryFileOperations fileOprn;
        MediaLibraryAlbumOperations albumOprn;
        MediaLibrarySmartAlbumOperations smartalbumOprn;
        MediaLibrarySmartAlbumMapOperations smartalbumMapOprn;
        string scanPath("");
        int32_t result(DATA_ABILITY_FAIL);
        string operationType = MediaLibraryDataAbilityUtils::GetOperationType(insertUri);
        MEDIA_INFO_LOG("operationType = %{public}s", operationType.c_str());
        if (insertUri.find(MEDIA_FILEOPRN) != string::npos) {
            result = fileOprn.HandleFileOperation(operationType, value, rdbStore);
            MEDIA_INFO_LOG("HandleFileOperation result = %{public}d", result);
            // After successfull close asset operation, do a scan file
            if ((result >= 0) && (operationType == MEDIA_FILEOPRN_CLOSEASSET)) {
                ScanFile(value, rdbStore);
            }
        } else if (insertUri.find(MEDIA_ALBUMOPRN) != string::npos) {
            result = albumOprn.HandleAlbumOperations(operationType, value, rdbStore);
        } else if (insertUri.find(MEDIA_SMARTALBUMOPRN) != string::npos) {
            result = smartalbumOprn.HandleSmartAlbumOperations(operationType, value, smartAlbumrdbStore);
        } else if (insertUri.find(MEDIA_SMARTALBUMMAPOPRN) != string::npos) {
            result = smartalbumMapOprn.HandleSmartAlbumMapOperations(operationType, value, smartAlbumMaprdbStore);
        }
        MEDIA_INFO_LOG("MediaLibraryDataAbility Insert: MEDIA_OPERN_KEYWORD END");
        return result;
    }

    // Normal URI scenario
    MEDIA_INFO_LOG("MediaLibraryDataAbility Insert: Normal URI scenario");
    int64_t outRowId = DATA_ABILITY_FAIL;
    (void)rdbStore->Insert(outRowId, MEDIALIBRARY_TABLE, value);
    MEDIA_INFO_LOG("no outRowId = %{public}lld", outRowId);
    MEDIA_INFO_LOG("MediaLibraryDataAbility Insert: END");
    return outRowId;
}

int32_t MediaLibraryDataAbility::Delete(const Uri &uri, const DataAbilityPredicates &predicates)
{
    if (!isRdbStoreInitialized || (rdbStore == nullptr)) {
        MEDIA_ERR_LOG("MediaLibraryDataAbility Delete:Rdb Store is not initialized");
        return DATA_ABILITY_FAIL;
    }

    string uriString = uri.ToString();

    string strDeleteCondition = predicates.GetWhereClause();
    if (strDeleteCondition.empty()) {
        string::size_type pos = uriString.find_last_of('/');
        CHECK_AND_RETURN_RET_LOG((pos != string::npos) && (pos == MEDIALIBRARY_DATA_URI.length()), DATA_ABILITY_FAIL,
            "Invalid index position");

        string strRow = uriString.substr(pos + 1);
        CHECK_AND_RETURN_RET_LOG(MediaLibraryDataAbilityUtils::IsNumber(strRow), DATA_ABILITY_FAIL, "Index not digit");

        strDeleteCondition = MEDIA_DATA_DB_ID + " = " + strRow;
        uriString = uriString.substr(0, pos);
    }

    // After removing the index values, check whether URI is correct
    CHECK_AND_RETURN_RET_LOG(uriString == MEDIALIBRARY_DATA_URI, DATA_ABILITY_FAIL, "Not Data ability Uri");

    vector<string> whereArgs = predicates.GetWhereArgs();
    int32_t deletedRows = DATA_ABILITY_FAIL;
    (void)rdbStore->Delete(deletedRows, MEDIALIBRARY_TABLE, strDeleteCondition, whereArgs);

    return deletedRows;
}
unique_ptr<AbsSharedResultSet> QueryBySmartTableType(TableType tabletype,
    string strQueryCondition,
    DataAbilityPredicates predicates,
    vector<string> columns,
    std::shared_ptr<NativeRdb::RdbStore> rdbStore)
{
    unique_ptr<AbsSharedResultSet> queryResultSet;
    if (tabletype == TYPE_SMARTALBUM) {
        AbsRdbPredicates mediaLibSAAbsPred(SMARTALBUM_TABLE);
        if (predicates.IsDistinct()) {
            mediaLibSAAbsPred.Distinct();
        }

        mediaLibSAAbsPred.SetWhereClause(strQueryCondition);
        mediaLibSAAbsPred.SetWhereArgs(predicates.GetWhereArgs());
        mediaLibSAAbsPred.Limit(predicates.GetLimit());
        mediaLibSAAbsPred.SetOrder(predicates.GetOrder());

        queryResultSet = rdbStore->Query(mediaLibSAAbsPred, columns);
        CHECK_AND_RETURN_RET_LOG(queryResultSet != nullptr, nullptr, "Query functionality failed");
    } else {
        AbsRdbPredicates mediaLibSAMAbsPred(SMARTALBUM_MAP_TABLE);
        if (predicates.IsDistinct()) {
            mediaLibSAMAbsPred.Distinct();
        }

        mediaLibSAMAbsPred.SetWhereClause(strQueryCondition);
        mediaLibSAMAbsPred.SetWhereArgs(predicates.GetWhereArgs());
        mediaLibSAMAbsPred.Limit(predicates.GetLimit());
        mediaLibSAMAbsPred.SetOrder(predicates.GetOrder());

        queryResultSet = rdbStore->Query(mediaLibSAMAbsPred, columns);
        CHECK_AND_RETURN_RET_LOG(queryResultSet != nullptr, nullptr, "Query functionality failed");
    }
    return queryResultSet;
}
unique_ptr<AbsSharedResultSet> QueryByFileTableType(TableType tabletype,
    string strQueryCondition,
    DataAbilityPredicates predicates,
    vector<string> columns,
    std::shared_ptr<NativeRdb::RdbStore> rdbStore)
{
    unique_ptr<AbsSharedResultSet> queryResultSet;
    if (tabletype == TYPE_ALBUM_TABLE) {
        AbsRdbPredicates mediaLibAbsPredAlbum(ABLUM_VIEW_NAME);
        if (strQueryCondition.empty()) {
            queryResultSet = rdbStore->QuerySql("SELECT * FROM " + ABLUM_VIEW_NAME);
        } else {
            if (predicates.IsDistinct()) {
                mediaLibAbsPredAlbum.Distinct();
            }
            mediaLibAbsPredAlbum.SetWhereClause(strQueryCondition);
            mediaLibAbsPredAlbum.SetWhereArgs(predicates.GetWhereArgs());
            mediaLibAbsPredAlbum.Limit(predicates.GetLimit());
            mediaLibAbsPredAlbum.SetOrder(predicates.GetOrder());
            queryResultSet = rdbStore->Query(mediaLibAbsPredAlbum, columns);
        }
    } else {
        AbsRdbPredicates mediaLibAbsPredFile(MEDIALIBRARY_TABLE);
        if (predicates.IsDistinct()) {
            mediaLibAbsPredFile.Distinct();
        }
        mediaLibAbsPredFile.SetWhereClause(strQueryCondition);
        mediaLibAbsPredFile.SetWhereArgs(predicates.GetWhereArgs());
        mediaLibAbsPredFile.Limit(predicates.GetLimit());
        mediaLibAbsPredFile.SetOrder(predicates.GetOrder());
        queryResultSet = rdbStore->Query(mediaLibAbsPredFile, columns);
    }
    return queryResultSet;
}
shared_ptr<AbsSharedResultSet> MediaLibraryDataAbility::Query(const Uri &uri,
                                                              const vector<string> &columns,
                                                              const DataAbilityPredicates &predicates)
{
    if ((!isRdbStoreInitialized) || (rdbStore == nullptr)) {
        MEDIA_ERR_LOG("MediaLibraryDataAbility Query:Rdb Store is not initialized");
        return nullptr;
    }

    string uriString = uri.ToString();
    unique_ptr<AbsSharedResultSet> queryResultSet;
    string strRow;
    TableType tabletype = TYPE_DATA;
    string strQueryCondition = predicates.GetWhereClause();
    if (uriString == MEDIALIBRARY_DATA_URI+"/"+MEDIA_ALBUMOPRN_QUERYALBUM) {
        tabletype = TYPE_ALBUM_TABLE;
        uriString = MEDIALIBRARY_DATA_URI;
        } else {
    if (strQueryCondition.empty()) {
        string::size_type pos = uriString.find_last_of('/');
        if (pos == MEDIALIBRARY_DATA_URI.length()) {
            strRow = uriString.substr(pos + 1);
            tabletype = TYPE_DATA;
            uriString = uriString.substr(0, pos);
            strQueryCondition = MEDIA_DATA_DB_ID + " = " + strRow;
        } else if (pos == MEDIALIBRARY_SMARTALBUM_URI.length()) {
            strRow = uriString.substr(pos + 1);
            tabletype = TYPE_SMARTALBUM;
            uriString = uriString.substr(0, pos);
            strQueryCondition = SMARTALBUM_DB_ID + " = " + strRow;
        } else if (pos == MEDIALIBRARY_SMARTALBUM_MAP_URI.length()) {
            strRow = uriString.substr(pos + 1);
            tabletype = TYPE_SMARTALBUM_MAP;
            uriString = uriString.substr(0, pos);
            strQueryCondition = SMARTALBUMMAP_DB_ID + " = " + strRow;
        }
    }
        }
    // After removing the index values, check whether URI is correct
    CHECK_AND_RETURN_RET_LOG((uriString == MEDIALIBRARY_DATA_URI ||
                             uriString == MEDIALIBRARY_SMARTALBUM_URI ||
                             uriString == MEDIALIBRARY_SMARTALBUM_MAP_URI),
                             nullptr,
                             "Not Data ability Uri");
    if (tabletype == TYPE_SMARTALBUM || tabletype == TYPE_SMARTALBUM_MAP) {
        queryResultSet = QueryBySmartTableType(tabletype, strQueryCondition, predicates, columns, rdbStore);
        CHECK_AND_RETURN_RET_LOG(queryResultSet != nullptr, nullptr, "Query functionality failed");
    } else {
        queryResultSet = QueryByFileTableType(tabletype, strQueryCondition, predicates, columns, rdbStore);
        CHECK_AND_RETURN_RET_LOG(queryResultSet != nullptr, nullptr, "Query functionality failed");
    }
    
    return queryResultSet;
}

int32_t MediaLibraryDataAbility::Update(const Uri &uri, const ValuesBucket &value,
    const DataAbilityPredicates &predicates)
{
    MEDIA_INFO_LOG("Update");
    if ((!isRdbStoreInitialized) || (rdbStore == nullptr) || (value.IsEmpty())) {
        MEDIA_ERR_LOG("MediaLibraryDataAbility Update:Input parameter is invalid ");
        return DATA_ABILITY_FAIL;
    }

    string uriString = uri.ToString();
    MEDIA_INFO_LOG("Update uriString = %{public}s", uriString.c_str());
    string strUpdateCondition = predicates.GetWhereClause();
    if (strUpdateCondition.empty()) {
        string::size_type pos = uriString.find_last_of('/');
        CHECK_AND_RETURN_RET_LOG((pos != string::npos) && (pos == MEDIALIBRARY_DATA_URI.length()), DATA_ABILITY_FAIL,
            "Invalid index position");

        string strRow = uriString.substr(pos + 1);
        CHECK_AND_RETURN_RET_LOG(MediaLibraryDataAbilityUtils::IsNumber(strRow), DATA_ABILITY_FAIL, "Index not digit");

        uriString = uriString.substr(0, pos);
        strUpdateCondition = MEDIA_DATA_DB_ID + " = " + strRow;
    }

    // After removing the index values, check whether URI is correct
    CHECK_AND_RETURN_RET_LOG(uriString == MEDIALIBRARY_DATA_URI, DATA_ABILITY_FAIL, "Not Data ability Uri");

    int32_t changedRows = DATA_ABILITY_FAIL;
    vector<string> whereArgs = predicates.GetWhereArgs();
    (void)rdbStore->Update(changedRows, MEDIALIBRARY_TABLE, value, strUpdateCondition, whereArgs);

    return changedRows;
}

int32_t MediaLibraryDataAbility::BatchInsert(const Uri &uri, const vector<ValuesBucket> &values)
{
    string uriString = uri.ToString();
    if ((!isRdbStoreInitialized) || (rdbStore == nullptr) || (uriString != MEDIALIBRARY_DATA_URI)) {
        MEDIA_ERR_LOG("MediaLibraryDataAbility BatchInsert: Input parameter is invalid");
        return DATA_ABILITY_FAIL;
    }
    int32_t rowCount = 0;
    for (auto it = values.begin(); it != values.end(); it++) {
        if (Insert(uri, *it) >= 0) {
            rowCount++;
        }
    }

    return rowCount;
}

void MediaLibraryDataAbility::ScanFile(const ValuesBucket &values, const shared_ptr<RdbStore> &rdbStore)
{
    if (scannerClient_ == nullptr) {
        scannerClient_ = MediaScannerHelperFactory::CreateScannerHelper();
    }

    if (scannerClient_ != nullptr) {
        string actualUri;
        ValueObject valueObject;

        if (values.GetObject(MEDIA_DATA_DB_URI, valueObject)) {
            valueObject.GetString(actualUri);
        }

        string id = MediaLibraryDataAbilityUtils::GetIdFromUri(actualUri);
        string srcPath = MediaLibraryDataAbilityUtils::GetPathFromDb(id, rdbStore);
        if (!srcPath.empty()) {
            std::shared_ptr<ScanFileCallback> scanFileCb = make_shared<ScanFileCallback>();
            CHECK_AND_RETURN_LOG(scanFileCb != nullptr, "Failed to create scan file callback object");
            auto ret = scannerClient_->ScanFile(srcPath, scanFileCb);
            CHECK_AND_RETURN_LOG(ret == 0, "Failed to initiate scan request");
        }
    }
}

/**
 * @brief 
 * 
 * @param uri 
 * @param  mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
 *  or "rwt" for read and write access that truncates any existing file.
 * / 
 * @return int32_t 
 */
int32_t MediaLibraryDataAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    string id = MediaLibraryDataAbilityUtils::GetIdFromUri(uri.ToString());

    shared_ptr<FileAsset> fileAsset = MediaLibraryDataAbilityUtils::GetFileAssetFromDb(id, rdbStore);
    CHECK_AND_RETURN_RET_LOG(fileAsset != nullptr, DATA_ABILITY_FAIL, "Failed to obtain path from Database");
    bool isWriteMode = MediaLibraryDataAbilityUtils::checkOpenMode(mode);

    if (isWriteMode && MediaLibraryDataAbilityUtils::checkFilePending(fileAsset)) {
        return DATA_ABILITY_HAS_OPENED_FAIL;
    }
    MEDIA_ERR_LOG("MediaLibraryDataAbility OpenAsset begin");
    int32_t fd = fileAsset->OpenAsset(fileAsset->GetPath(), mode);
    MEDIA_ERR_LOG("MediaLibraryDataAbility OpenAsset end");

    if (isWriteMode && fd > 0) {
        int32_t errorCode = MediaLibraryDataAbilityUtils::setFilePending(fileAsset->GetId(), true, rdbStore);
        if (errorCode == DATA_ABILITY_FAIL) {
            fileAsset->CloseAsset(fd);
            MEDIA_ERR_LOG("MediaLibraryDataAbility OpenFile: Set file to pending DB error");
            return DATA_ABILITY_HAS_DB_ERROR;
        }
    }
    MEDIA_ERR_LOG("MediaLibraryDataAbility OpenFile: Success");
    return fd;
}

void ScanFileCallback::OnScanFinished(const int32_t status, const std::string &uri, const std::string &path) {}
}  // namespace Media
}  // namespace OHOS