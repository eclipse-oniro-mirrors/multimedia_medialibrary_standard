/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "medialibrary_rdb_test.h"
#include "medialibrary_dir_operations.h"
#include "medialibrary_unistore_manager.h"
#include "ability_context_impl.h"
#include "rdb_utils.h"
#include "uri.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {

HWTEST_F(MediaLibraryExtUnitTest, medialib_HandleDirOperation_test_001, TestSize.Level0)
{
    MediaLibraryCommand cmd(OperationObject::FILESYSTEM_ASSET, OperationType::CREATE);
    int32_t ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    MediaLibraryCommand cmd1(OperationObject::FILESYSTEM_ASSET, OperationType::TRASH);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd1);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    MediaLibraryCommand cmd2(OperationObject::FILESYSTEM_ASSET, OperationType::UNKNOWN_TYPE);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd2);
    EXPECT_EQ(ret, E_FAIL);
}

HWTEST_F(MediaLibraryExtUnitTest, medialib_HandleDirOperation_test_002, TestSize.Level0)
{
    MediaLibraryCommand cmd(OperationObject::FILESYSTEM_ASSET, OperationType::CREATE);
    int32_t ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    auto context = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    MediaLibraryUnistoreManager::GetInstance().Init(context);
    ValuesBucket values;
    cmd.SetValueBucket(values);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    values.PutString(MEDIA_DATA_DB_RELATIVE_PATH, "medialib_HandleDirOperation_testCase_002");
    cmd.SetValueBucket(values);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ((ret > E_OK || ret == E_FILE_EXIST), true);
    MediaLibraryUnistoreManager::GetInstance().Stop();
}

HWTEST_F(MediaLibraryExtUnitTest, medialib_HandleDirOperation_test_003, TestSize.Level0)
{
    MediaLibraryCommand cmd(OperationObject::FILESYSTEM_ASSET, OperationType::TRASH);
    int32_t ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    auto context = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    MediaLibraryUnistoreManager::GetInstance().Init(context);
    ValuesBucket valuesBucket;
    cmd.SetValueBucket(valuesBucket);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    valuesBucket.PutString(MEDIA_DATA_DB_RELATIVE_PATH, "medialib_handleTestCase_01");
    valuesBucket.PutInt(MEDIA_DATA_DB_ID, 0);
    cmd.SetValueBucket(valuesBucket);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_GET_VALUEBUCKET_FAIL);

    MediaLibraryCommand cmd1(OperationObject::FILESYSTEM_ASSET, OperationType::TRASH);
    ValuesBucket values;
    values.PutString(MEDIA_DATA_DB_RELATIVE_PATH, "medialib_handleTestCase_02");
    values.PutInt(MEDIA_DATA_DB_ID, 10);
    cmd1.SetValueBucket(values);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd1);
    EXPECT_EQ(ret, E_GET_ASSET_FAIL);

    MediaLibraryUnistoreManager::GetInstance().Stop();
}

HWTEST_F(MediaLibraryExtUnitTest, medialib_HandleDirOperation_test_004, TestSize.Level0)
{
    MediaLibraryCommand cmd(OperationObject::FILESYSTEM_ASSET, OperationType::UNKNOWN_TYPE);
    int32_t ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_FAIL);

    auto context = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    MediaLibraryUnistoreManager::GetInstance().Init(context);
    ValuesBucket values;
    values.PutString(MEDIA_DATA_DB_RELATIVE_PATH, "medialib_HandleDirOperation_test_004");
    values.PutInt(MEDIA_DATA_DB_ID, 11);
    cmd.SetValueBucket(values);
    ret = MediaLibraryDirOperations::HandleDirOperation(cmd);
    EXPECT_EQ(ret, E_FAIL);
    MediaLibraryUnistoreManager::GetInstance().Stop();
}

HWTEST_F(MediaLibraryExtUnitTest, medialib_CreateDirOperation_test_001, TestSize.Level0)
{
    string queryUri = MEDIALIBRARY_DATA_URI;
    Uri uri(queryUri);
    MediaLibraryCommand cmd(uri, OperationType::QUERY);
    int32_t ret = MediaLibraryDirOperations::CreateDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    auto context = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    MediaLibraryUnistoreManager::GetInstance().Init(context);
    ValuesBucket values;
    cmd.SetValueBucket(values);
    ret = MediaLibraryDirOperations::CreateDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    values.PutString(MEDIA_DATA_DB_RELATIVE_PATH, "medialib_CreateDirOperation_test_001");
    cmd.SetValueBucket(values);
    ret = MediaLibraryDirOperations::CreateDirOperation(cmd);
    EXPECT_EQ((ret > E_OK || ret == E_FILE_EXIST), true);
    MediaLibraryUnistoreManager::GetInstance().Stop();
}

HWTEST_F(MediaLibraryExtUnitTest, medialib_TrashDirOperation_test_001, TestSize.Level0)
{
    string queryUri = MEDIALIBRARY_DATA_URI;
    Uri uri(queryUri);
    MediaLibraryCommand cmd(uri, OperationType::QUERY);
    int32_t ret = MediaLibraryDirOperations::TrashDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    auto context = std::make_shared<OHOS::AbilityRuntime::AbilityContextImpl>();
    MediaLibraryUnistoreManager::GetInstance().Init(context);
    ValuesBucket values;
    cmd.SetValueBucket(values);
    ret = MediaLibraryDirOperations::TrashDirOperation(cmd);
    EXPECT_EQ(ret, E_HAS_DB_ERROR);

    values.PutString(MEDIA_DATA_DB_RELATIVE_PATH, "medialib_TrashTestCase_01");
    values.PutInt(MEDIA_DATA_DB_ID, 0);
    cmd.SetValueBucket(values);
    ret = MediaLibraryDirOperations::TrashDirOperation(cmd);
    EXPECT_EQ(ret, E_GET_VALUEBUCKET_FAIL);

    MediaLibraryCommand cmd1(uri, OperationType::QUERY);
    ValuesBucket valuesBucket;
    valuesBucket.PutString(MEDIA_DATA_DB_RELATIVE_PATH, "medialib_TrashTestCase_02");
    valuesBucket.PutInt(MEDIA_DATA_DB_ID, 12);
    cmd1.SetValueBucket(valuesBucket);
    ret = MediaLibraryDirOperations::TrashDirOperation(cmd1);
    EXPECT_EQ(ret, E_GET_ASSET_FAIL);
    MediaLibraryUnistoreManager::GetInstance().Stop();
}

}
}