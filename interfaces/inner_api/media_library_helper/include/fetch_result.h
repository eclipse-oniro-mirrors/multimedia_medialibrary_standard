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

#ifndef INTERFACES_INNERKITS_NATIVE_INCLUDE_FETCH_RESULT_H_
#define INTERFACES_INNERKITS_NATIVE_INCLUDE_FETCH_RESULT_H_

#include <variant>
#include "abs_shared_result_set.h"
#include "datashare_result_set.h"
#include "file_asset.h"
#include "medialibrary_type_const.h"
#include "medialibrary_db_const.h"
#include "rdb_errno.h"
#include "userfile_manager_types.h"

namespace OHOS {
namespace Media {
enum ResultSetDataType {
    TYPE_STRING,
    TYPE_INT32,
    TYPE_INT64
};

/**
 * @brief Class for returning the data cursor to application.
 *
 * @since 1.0
 * @version 1.0
 */
class FetchResult {
public:
    explicit FetchResult(const std::shared_ptr<DataShare::DataShareResultSet>& resultset);
    FetchResult();
    virtual ~FetchResult();

    void Close();
    int32_t GetCount();
    bool IsContain();
    bool IsClosed();
    bool IsAtLastRow();
    std::unique_ptr<FileAsset> GetObjectAtPosition(int32_t index);
    std::unique_ptr<FileAsset> GetFirstObject();
    std::unique_ptr<FileAsset> GetObjectFromRdb(std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet, int idx);
    std::unique_ptr<FileAsset> GetNextObject();
    std::unique_ptr<FileAsset> GetLastObject();
    std::unique_ptr<FileAsset> GetObject();

    bool isContain_;
    bool isClosed_;
    int32_t count_;
    std::string networkId_;
    ResultNapiType resultNapiType_;
    std::shared_ptr<OHOS::DataShare::DataShareResultSet> resultset_ = nullptr;

private:
    std::unique_ptr<FileAsset> GetObject(std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet);
    std::variant<int32_t, int64_t, std::string> GetRowValFromColumn(std::string columnName,
        ResultSetDataType dataType, std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet);
};
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_INNERKITS_NATIVE_INCLUDE_FETCH_RESULT_H_
