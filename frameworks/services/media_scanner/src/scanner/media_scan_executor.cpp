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

#include "media_scan_executor.h"
#include <thread>

namespace OHOS {
namespace Media {
using namespace std;

int32_t MediaScanExecutor::Commit(std::unique_ptr<MediaScannerObj> scanner)
{
    lock_guard<mutex> lock(queueMutex_);

    queue_.push(move(scanner));

    if (activeThread_ < MAX_THREAD) {
        thread(&MediaScanExecutor::HandleScanExecution, this).detach();
        activeThread_++;
    }

    return 0;
}

void MediaScanExecutor::HandleScanExecution()
{
    unique_ptr<MediaScannerObj> scanner;
    while (true) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (queue_.empty()) {
                activeThread_--;
                break;
            }

            scanner = std::move(queue_.front());
            queue_.pop();
        }

        if (scanner->isDir()) {
            scanner->ScanDir();
        } else {
            scanner->ScanFile();
        }
    }
}
} // namespace Media
} // namespace OHOS
