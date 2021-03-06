/*
 * Copyright 2016 Gigatribe
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GIGA_API_FILEUPLOADER_H_
#define GIGA_API_FILEUPLOADER_H_

#include "FileTransferer.h"

#include <pplx/pplxtasks.h>
#include <cpprest/details/basic_types.h>
#include <boost/filesystem.hpp>

namespace giga
{
class Application;

namespace core
{
class Node;
class UploadState;

/**
 * Upload a file.
 *
 * REMEMBER: Before uploading a file, we need to calculate its sha1.
 * It is a long running process, that we call the preparing phase.
 *
 * After the preparing phase, we test to see if the file already exists in GiGa.GG.
 * If it does, the uploading process is skipped (we use the one already in GiGa.GG)
 */
class FileUploader final : public FileTransferer
{
public:
    explicit
    FileUploader (const boost::filesystem::path& filename, const utility::string_t& nodeName, const std::string& parentId,
                  const std::string& sha1, const std::string& fid, const std::string& fkey, const Application& app,
                  pplx::cancellation_token_source cts = pplx::cancellation_token_source{});
    virtual ~FileUploader ();

    FileUploader ()                              = delete;
    FileUploader (FileUploader&&)                = delete;
    FileUploader& operator=(const FileUploader&) = delete;
    FileUploader (const FileUploader&)           = delete;

public:

    /**
     * @brief Gets the task managing the upload process.
     * Make sure you have started the upload before calling this method.
     * @see FileTransferer::start()
     * @see https://github.com/Microsoft/cpprestsdk
     * @see http://microsoft.github.io/cpprestsdk/classpplx_1_1task.html
     */
    const pplx::task<std::shared_ptr<Node>>&
    task () const;

    /**
     * @brief Gets this upload progress.
     */
    FileTransferer::Progress
    progress () const override;

    const utility::string_t&
    nodeName() const;

    const boost::filesystem::path&
    filename() const override;

    uint64_t
    fileSize() const;

protected:
    void
    doStart () override;

private:
    pplx::task<std::shared_ptr<Node>> _task;

    boost::filesystem::path _filename;
    utility::string_t       _nodeName;
    std::string             _parentId;
    std::string             _sha1;
    std::string             _fid;
    std::string             _fkey;

    uint64_t           _fileSize;
    uint64_t           _fileCDate;
    const Application* _app;
};

} /* namespace api */
} /* namespace giga */

#endif /* GIGA_API_FILEUPLOADER_H_ */
