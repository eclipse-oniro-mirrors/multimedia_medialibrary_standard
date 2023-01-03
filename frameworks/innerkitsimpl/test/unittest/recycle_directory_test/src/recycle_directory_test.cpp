/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "recycle_directory_test.h"

#include <unistd.h>

#include "get_self_permissions.h"
#include "iservice_registry.h"
#include "media_log.h"
#include "media_library_manager.h"
#include "medialibrary_db_const.h"
#include "medialibrary_data_manager.h"
#include "medialibrary_object_utils.h"
#include "medialibrary_unittest_utils.h"
#include "media_thumbnail_helper.h"
#include "result_set_utils.h"
#include "scanner_utils.h"

using namespace std;
using namespace OHOS::NativeRdb;
using namespace testing::ext;

namespace OHOS {
namespace Media {
static constexpr int32_t g_stashAlbumID = 2;
void RecycleDirectory::SetUpTestCase() {
    MEDIA_INFO_LOG("RecycleDirectory::SetUpTestCase:: invoked");
    MediaLibraryUnitTestUtils::Init();
    MEDIA_INFO_LOG("RecycleDirectory::SetUpTestCase:: Finish");
}
void RecycleDirectory::TearDownTestCase() {}
void RecycleDirectory::SetUp() {}
void RecycleDirectory::TearDown(void) {}

static int32_t GetAlbumId(std::string relativePath)
{
    return MediaLibraryObjectUtils::GetIdByPathFromDb("/storage/media/local/files/" + relativePath);
}

static int32_t CreateFile(std::string relativePath, std::string displayName)
{
    string abilityUri = Media::MEDIALIBRARY_DATA_URI;
    Uri createAssetUri(abilityUri + "/" + Media::MEDIA_FILEOPRN + "/" +
		Media::MEDIA_FILEOPRN_CREATEASSET);
    DataShareValuesBucket valuesBucket;

    valuesBucket.Put(MEDIA_DATA_DB_MEDIA_TYPE, MEDIA_TYPE_IMAGE);
    valuesBucket.Put(MEDIA_DATA_DB_NAME, displayName);
    valuesBucket.Put(MEDIA_DATA_DB_RELATIVE_PATH, relativePath);
    int32_t res = MediaLibraryDataManager::GetInstance()->Insert(createAssetUri, valuesBucket);
    if (res <= 0) {
        MEDIA_ERR_LOG("Failed to create file, error: %{public}d", res);
    }
    return res;
}

static int32_t CreateAlbum(std::string relativePath)
{
    string abilityUri = Media::MEDIALIBRARY_DATA_URI;
    Uri createAssetUri(abilityUri + "/" + Media::MEDIA_DIROPRN + "/" +
		Media::MEDIA_DIROPRN_FMS_CREATEDIR);
    DataShareValuesBucket valuesBucket;
    valuesBucket.Put(MEDIA_DATA_DB_RELATIVE_PATH, relativePath);
    int32_t res = MediaLibraryDataManager::GetInstance()->Insert(createAssetUri, valuesBucket);
    if (res <= 0) {
        MEDIA_ERR_LOG("Failed to create album, error: %{public}d", res);
    }
    return res;
}

static void AddSmartAlbum(int32_t id)
{
    DataShareValuesBucket valuesBucket;
    valuesBucket.Put(SMARTALBUMMAP_DB_ALBUM_ID, g_stashAlbumID);
    valuesBucket.Put(SMARTALBUMMAP_DB_CHILD_ASSET_ID, id);
    Uri AddAsseturi(MEDIALIBRARY_DATA_URI + "/"
    + MEDIA_SMARTALBUMMAPOPRN + "/" + MEDIA_SMARTALBUMMAPOPRN_ADDSMARTALBUM);
    MediaLibraryDataManager::GetInstance()->Insert(AddAsseturi, valuesBucket);
}

static int32_t RemoveSmartAlbum(int32_t id)
{
    DataShareValuesBucket valuesBucket;
    valuesBucket.Put(SMARTALBUMMAP_DB_ALBUM_ID, g_stashAlbumID);
    valuesBucket.Put(SMARTALBUMMAP_DB_CHILD_ASSET_ID, id);
    Uri RemoveAsseturi(MEDIALIBRARY_DATA_URI + "/" + MEDIA_SMARTALBUMMAPOPRN +
		"/" + MEDIA_SMARTALBUMMAPOPRN_REMOVESMARTALBUM);
    return MediaLibraryDataManager::GetInstance()->Insert(RemoveAsseturi, valuesBucket);
}

/**
 * @tc.number    : delete_directory_001
 * @tc.name      : FMS create_delete_directory
 * @tc.desc      : 1. create file Pictures/test001/gtest_001.jpg
 *                 2. push albumId into the recycle
 *                 3. pop albumId out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_001, TestSize.Level0)
{
    MEDIA_INFO_LOG("recycle_directory_001::Start");
    int32_t fileId = CreateFile("Pictures/test001/", "gtest_001.jpg");
    ASSERT_GT(fileId, 0);
    int32_t albumId = GetAlbumId("Pictures/test001");
    AddSmartAlbum(albumId);
    int32_t changedRows = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows < 0), true);
    MEDIA_INFO_LOG("recycle_directory_001::End");
}

/**
 * @tc.number    : recycle_directory_002
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test002/test002/gtest_002.jpg
 *                 2. push albumId into the recycle
 *                 3. pop albumId out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_002, TestSize.Level0)
{
    MEDIA_INFO_LOG("recycle_directory_002::Start");
    int32_t fileId_01 = CreateFile("Pictures/test002/", "gtest_002.jpg");
    int32_t fileId_02 = CreateFile("Pictures/test002/test002/", "gtest_002.jpg");
    ASSERT_GT(fileId_01, 0);
    ASSERT_GT(fileId_02, 0);
    int32_t albumId = GetAlbumId("Pictures/test002");
    AddSmartAlbum(albumId);
    int32_t changedRows = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows < 0), true);
    MEDIA_INFO_LOG("recycle_directory_002::End");
}

/**
 * @tc.number    : recycle_directory_003
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test003/gtest_003.jpg
 *                 2. push file into the recycle
 *                 3. push albumId into the recycle
 *                 4. pop file out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_003, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test003/", "gtest_003.jpg");
    ASSERT_GT(fileId, 0);
    int32_t albumId = GetAlbumId("Pictures/test003");
    AddSmartAlbum(fileId);
    AddSmartAlbum(albumId);
    int32_t changedRows = RemoveSmartAlbum(fileId);
    ASSERT_NE((changedRows < 0), true);
    MEDIA_INFO_LOG("recycle_directory_003::End");
}

/**
 * @tc.number    : recycle_directory_004
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test004/gtest_004.jpg
 *                 2. push file into the recycle
 *                 3. push albumId into the recycle
 *                 4. pop file out of the recycle
 *                 5. pop albumId out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_004, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test004/", "gtest_004.jpg");
    ASSERT_GT(fileId, 0);
    int32_t albumId = GetAlbumId("Pictures/test004");
    AddSmartAlbum(fileId);
    AddSmartAlbum(albumId);
    int32_t changedRows1 = RemoveSmartAlbum(fileId);
    int32_t changedRows2 = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows1 < 0), true);
    ASSERT_NE((changedRows2 < 0), true);
    MEDIA_INFO_LOG("recycle_directory_004::End");
}
/**
 * @tc.number    : recycle_directory_005
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test005/gtest_005.jpg
 *                 2. push albumId into the recycle
 *                 3. create file Pictures/test005/
 *                 4. pop albumId out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_005, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test005/", "gtest_005.jpg");
    ASSERT_GT(fileId, 0);
    int32_t albumId = GetAlbumId("Pictures/test005");
    AddSmartAlbum(albumId);
    int32_t albumIdSame = CreateAlbum("Pictures/test005/");
    ASSERT_GT(albumIdSame, 0);
    int32_t changedRows = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows < 0), true);
    MEDIA_INFO_LOG("recycle_directory_005::End");
}
/**
 * @tc.number    : recycle_directory_006
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test006/gtest_006.jpg
 *                 2. push albumId into the recycle
 *                 3. create album Pictures/test006/
 *                 4. create album Pictures/test006_recycle/
 *                 5. pop albumId out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_006, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test006/", "gtest_006.jpg");
    ASSERT_GT(fileId, 0);
    MEDIA_INFO_LOG("fileId::%d\n", fileId);
    int32_t albumId = GetAlbumId("Pictures/test006");
    AddSmartAlbum(albumId);
    int32_t albumIdSame = CreateAlbum("Pictures/test006/");
    ASSERT_GT(albumIdSame, 0);
    int32_t albumIdSameRecycle = CreateAlbum("Pictures/test006_recycle/");
    ASSERT_GT(albumIdSameRecycle, 0);
    int32_t changedRows = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows < 0), true);
    MEDIA_INFO_LOG("recycle_directory_006::End");
}
/**
 * @tc.number    : recycle_directory_007
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test007/gtest_007.jpg
 *                 2. push albumId into the recycle
 *                 3. create file Pictures/test007/gtest_007.jpg
 *                 4. pop albumId out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_007, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test007/", "gtest_007.jpg");
    int32_t albumId = GetAlbumId("Pictures/test007");
    AddSmartAlbum(albumId);
    ASSERT_GT(fileId, 0);
    int32_t fileIdSame = CreateFile("Pictures/test007/", "gtest_007.jpg");
    ASSERT_GT(fileIdSame, 0);
    int32_t albumIdSame = GetAlbumId("Pictures/test007");
    ASSERT_GT(albumIdSame, 0);
    int32_t changedRows2 = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows2 < 0), true);
    MEDIA_INFO_LOG("recycle_directory_007::End");
}
/**
 * @tc.number    : recycle_directory_008
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test008/gtest_008.jpg
 *                 2. push albumId into the recycle
 *                 3. pop albumId out of the recycle
 *                 4. push albumId into the recycle
 *                 5. pop albumId out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_008, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test008/", "gtest_008.jpg");
    ASSERT_GT(fileId, 0);
    int32_t albumId = GetAlbumId("Pictures/test008");
    AddSmartAlbum(albumId);
    int32_t changedRows1 = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows1 < 0), true);
    AddSmartAlbum(albumId);
    int32_t changedRows2 = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows2 < 0), true);
    MEDIA_INFO_LOG("recycle_directory_008::End");
}
/**
 * @tc.number    : recycle_directory_009
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test009/gtest_009.jpg
 *                 2. push fileId into the recycle
 *                 3. push album into the recycle
 *                 4. pop fileId out of the recycle
 *                 5. push file into the recycle
 *                 6. pop file out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_009, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test009/", "gtest_009.jpg");
    ASSERT_GT(fileId, 0);
    int32_t albumId = GetAlbumId("Pictures/test009");
    AddSmartAlbum(fileId);
    AddSmartAlbum(albumId);
    int32_t changedRows1 = RemoveSmartAlbum(fileId);
    ASSERT_NE((changedRows1 < 0), true);
    AddSmartAlbum(fileId);
    int32_t changedRows2 = RemoveSmartAlbum(fileId);
    ASSERT_NE((changedRows2 < 0), true);
    MEDIA_INFO_LOG("recycle_directory_009::End");
}
/**
 * @tc.number    : recycle_directory_010
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test010/gtest_010.jpg
 *                 2. push fileId into the recycle
 *                 3. push album into the recycle
 *                 4. pop fileId out of the recycle
 *                 5. push file into the recycle
 *                 6. pop file out of the recycle
 *                 7. pop album out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_010, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test010/", "gtest_010.jpg");
    ASSERT_GT(fileId, 0);
    int32_t albumId = GetAlbumId("Pictures/test010");
    AddSmartAlbum(fileId);
    AddSmartAlbum(albumId);
    int32_t changedRows = RemoveSmartAlbum(fileId);
    ASSERT_NE((changedRows < 0), true);
    AddSmartAlbum(fileId);
    int32_t changedRows1 = RemoveSmartAlbum(fileId);
    int32_t changedRows2 = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows1 < 0), true);
    ASSERT_NE((changedRows2 < 0), true);
    MEDIA_INFO_LOG("recycle_directory_010::End");
}
/**
 * @tc.number    : recycle_directory_011
 * @tc.name      : recycle_directory
 * @tc.desc      : 1. create file Pictures/test011/01.jpg
 *                 2. create file Pictures/test011/01/01.jpg
 *                 3. push Pictures/test011/01 into the recycle
 *                 4. push Pictures/test011 into the recycle
 *                 5. pop Pictures/test011/01 out of the recycle
 *                 6. pop Pictures/test011 out of the recycle
 */
HWTEST_F(RecycleDirectory, recycle_directory_011, TestSize.Level0)
{
    int32_t fileId = CreateFile("Pictures/test011/", "gtest011.jpg");
    int32_t fileId_01 = CreateFile("Pictures/test011/01/", "01.jpg");
    ASSERT_GT(fileId, 0);
    ASSERT_GT(fileId_01, 0);
    int32_t albumId_01 = GetAlbumId("Pictures/test011/01");
    AddSmartAlbum(albumId_01);
    int32_t albumId = GetAlbumId("Pictures/test011");
    AddSmartAlbum(albumId);
    int32_t changedRows1 = RemoveSmartAlbum(albumId_01);
    int32_t changedRows2 = RemoveSmartAlbum(albumId);
    ASSERT_NE((changedRows1 < 0), true);
    ASSERT_NE((changedRows2 < 0), true);
    MEDIA_INFO_LOG("recycle_directory_011::End");
}

} // namespace Media
} // namespace OHOS