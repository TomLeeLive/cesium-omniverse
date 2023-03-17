#include "cesium/omniverse/HttpAssetAccessor.h"

#include "cesium/omniverse/LoggerSink.h"

#include <zlib.h>

namespace cesium::omniverse {
namespace {
std::string decodeGzip(std::string& content) {
    z_stream zs; // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, MAX_WBITS + 16) != Z_OK) {
        return std::move(content);
    }

    zs.next_in = reinterpret_cast<Bytef*>(content.data());
    zs.avail_in = static_cast<uInt>(content.size());

    int ret;
    char outbuffer[32768];
    std::string output;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (output.size() < zs.total_out) {
            std::size_t decompressSoFar = output.size();
            std::size_t addSize = zs.total_out - output.size();
            output.resize(output.size() + addSize);
            std::memcpy(output.data() + decompressSoFar, outbuffer, addSize);
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        return std::move(content);
    }

    return output;
}

struct GZipDecompressInterceptor : public cpr::Interceptor {
  public:
    GZipDecompressInterceptor(const std::filesystem::path& certificatePath)
        : _certificatePath(certificatePath.generic_string()) {}

    cpr::Response intercept(cpr::Session& session) override {
#ifdef CESIUM_OMNI_UNIX
        curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_CAINFO, _certificatePath.c_str());
#endif
        curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_ACCEPT_ENCODING, nullptr);

        CURLcode curl_error = curl_easy_perform(session.GetCurlHolder()->handle);

        if (curl_error == CURLE_PEER_FAILED_VERIFICATION) {
            long verifyResult;
            curl_easy_getinfo(session.GetCurlHolder()->handle, CURLINFO_SSL_VERIFYRESULT, &verifyResult);
            CESIUM_LOG_WARN("SSL PEER VERIFICATION FAILED: {}", verifyResult);
        }

        auto response = session.Complete(curl_error);
        response.text = decodeGzip(response.text);
        return response;
    }

  private:
    std::string _certificatePath;
};

struct HeaderInterceptor : public cpr::Interceptor {
  public:
    HeaderInterceptor(const std::string& version, const std::string& project)
        : _version(version)
        , _project(project) {}

    cpr::Response intercept(cpr::Session& session) override {
        cpr::Header xCesiumHeaders = {};
        xCesiumHeaders.insert(std::make_pair("X-Cesium-Client", "Cesium for Omniverse"));
        xCesiumHeaders.insert(std::make_pair("X-Cesium-Client-Version", _version));

        if (!_project.empty()) {
            xCesiumHeaders.insert(std::make_pair("X-Cesium-Client-Project", _project));
        }

        xCesiumHeaders.insert(std::make_pair("X-Cesium-Client-Engine", "Cesium for Omniverse"));
        xCesiumHeaders.insert(std::make_pair("X-Cesium-Client-Engine", "Cesium for Omniverse"));
        return proceed(session);
    }

  private:
    std::string _version;
    std::string _project;
};
} // namespace

HttpAssetAccessor::HttpAssetAccessor(const std::filesystem::path& certificatePath) {
    _interceptor = std::make_shared<GZipDecompressInterceptor>(certificatePath);
}

CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> HttpAssetAccessor::get(
    const CesiumAsync::AsyncSystem& asyncSystem,
    const std::string& url,
    const std::vector<THeader>& headers) {
    auto promise = asyncSystem.createPromise<std::shared_ptr<CesiumAsync::IAssetRequest>>();
    cpr::Header cprHeaders{headers.begin(), headers.end()};
    std::shared_ptr<cpr::Session> session = std::make_shared<cpr::Session>();
    session->AddInterceptor(_interceptor);
    session->SetHeader(cprHeaders);
    session->SetUrl(cpr::Url(url));
    session->GetCallback([promise, url, headers](cpr::Response&& response) mutable {
        promise.resolve(
            std::make_shared<HttpAssetRequest>("GET", std::move(url), std::move(headers), std::move(response)));
    });

    return promise.getFuture();
}

CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> HttpAssetAccessor::request(
    const CesiumAsync::AsyncSystem& asyncSystem,
    const std::string& verb,
    const std::string& url,
    const std::vector<THeader>& headers,
    const gsl::span<const std::byte>& contentPayload) {
    auto promise = asyncSystem.createPromise<std::shared_ptr<CesiumAsync::IAssetRequest>>();
    cpr::Header cprHeaders{headers.begin(), headers.end()};
    std::shared_ptr<cpr::Session> session = std::make_shared<cpr::Session>();
    session->SetHeader(cprHeaders);
    session->SetUrl(cpr::Url(url));
#ifdef CESIUM_OMNI_UNIX
    session->AddInterceptor(_interceptor);
#endif
    if (verb == "GET") {
        session->GetCallback([promise, url, headers](cpr::Response&& response) mutable {
            promise.resolve(
                std::make_shared<HttpAssetRequest>("GET", std::move(url), std::move(headers), std::move(response)));
        });

        return promise.getFuture();
    } else if (verb == "POST") {
        session->SetBody(cpr::Body{reinterpret_cast<const char*>(contentPayload.data()), contentPayload.size()});
        session->PostCallback([promise, url, headers](cpr::Response&& response) mutable {
            promise.resolve(
                std::make_shared<HttpAssetRequest>("POST", std::move(url), std::move(headers), std::move(response)));
        });

        return promise.getFuture();
    }

    return asyncSystem.createResolvedFuture<std::shared_ptr<CesiumAsync::IAssetRequest>>(nullptr);
}

void HttpAssetAccessor::tick() noexcept {}
} // namespace cesium::omniverse
