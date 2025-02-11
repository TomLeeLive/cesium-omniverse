// Copyright 2023 CesiumGS, Inc. and Contributors

#include "cesium/omniverse/CesiumIonSession.h"

#include "cesium/omniverse/Broadcast.h"
#include "cesium/omniverse/SettingsWrapper.h"

#include <utility>

using namespace CesiumAsync;
using namespace CesiumIonClient;

using namespace cesium::omniverse;

#ifdef CESIUM_OMNI_WINDOWS
const char* browserCommandBase = "start";
#else
const char* browserCommandBase = "xdg-open";
#endif

CesiumIonSession::CesiumIonSession(
    CesiumAsync::AsyncSystem& asyncSystem,
    std::shared_ptr<CesiumAsync::IAssetAccessor> pAssetAccessor,
    std::string ionApiUrl,
    int64_t ionApplicationId)
    : _asyncSystem(asyncSystem)
    , _pAssetAccessor(std::move(pAssetAccessor))
    , _connection(std::nullopt)
    , _profile(std::nullopt)
    , _assets(std::nullopt)
    , _tokens(std::nullopt)
    , _isConnecting(false)
    , _isResuming(false)
    , _isLoadingProfile(false)
    , _isLoadingAssets(false)
    , _isLoadingTokens(false)
    , _loadProfileQueued(false)
    , _loadAssetsQueued(false)
    , _loadTokensQueued(false)
    , _authorizeUrl()
    , _ionApiUrl(std::move(ionApiUrl))
    , _ionApplicationId(ionApplicationId) {}

void CesiumIonSession::connect() {
    if (this->isConnecting() || this->isConnected() || this->isResuming()) {
        return;
    }

    this->_isConnecting = true;

    Connection::authorize(
        this->_asyncSystem,
        this->_pAssetAccessor,
        "Cesium for Omniverse",
        _ionApplicationId,
        "/cesium-for-omniverse/oauth2/callback",
        {"assets:list", "assets:read", "profile:read", "tokens:read", "tokens:write", "geocode"},
        [this](const std::string& url) {
            // NOTE: We open the browser in the Python code. Check in the sign in widget's on_update_frame function.
            this->_authorizeUrl = url;
        })
        .thenInMainThread([this](CesiumIonClient::Connection&& connection) {
            this->_isConnecting = false;
            this->_connection = std::move(connection);

            Settings::UserAccessToken token;
            token.ionApiUrl = _ionApiUrl;
            token.token = this->_connection.value().getAccessToken();
            Settings::setAccessToken(token);

            Broadcast::connectionUpdated();
        })
        .catchInMainThread([this]([[maybe_unused]] std::exception&& e) {
            this->_isConnecting = false;
            this->_connection = std::nullopt;

            Broadcast::connectionUpdated();
        });
}

void CesiumIonSession::resume() {
    if (this->isConnecting() || this->isConnected() || this->isResuming()) {
        return;
    }

    auto tokens = Settings::getAccessTokens();

    if (tokens.size() < 1) {
        // No existing session to resume.
        return;
    }

    std::string userAccessToken;
    for (const auto& token : tokens) {
        if (token.ionApiUrl == _ionApiUrl) {
            userAccessToken = token.token;
            break;
        }
    }

    if (userAccessToken.empty()) {
        // No existing session to resume.
        return;
    }

    this->_isResuming = true;

    this->_connection = Connection(this->_asyncSystem, this->_pAssetAccessor, userAccessToken);

    // Verify that the connection actually works.
    this->_connection.value()
        .me()
        .thenInMainThread([this](Response<Profile>&& response) {
            if (!response.value.has_value()) {
                this->_connection.reset();
            }
            this->_isResuming = false;
            Broadcast::connectionUpdated();
        })
        .catchInMainThread([this]([[maybe_unused]] std::exception&& e) {
            this->_isResuming = false;
            this->_connection.reset();
            Broadcast::connectionUpdated();
        });
}

void CesiumIonSession::disconnect() {
    this->_connection.reset();
    this->_profile.reset();
    this->_assets.reset();
    this->_tokens.reset();

    Settings::removeAccessToken(_ionApiUrl);

    Broadcast::connectionUpdated();
    Broadcast::profileUpdated();
    Broadcast::assetsUpdated();
    Broadcast::tokensUpdated();
}

void CesiumIonSession::tick() {
    this->_asyncSystem.dispatchMainThreadTasks();
}

void CesiumIonSession::refreshProfile() {
    if (!this->_connection || this->_isLoadingProfile) {
        this->_loadProfileQueued = true;
        return;
    }

    this->_isLoadingProfile = true;
    this->_loadProfileQueued = false;

    this->_connection->me()
        .thenInMainThread([this](Response<Profile>&& profile) {
            this->_isLoadingProfile = false;
            this->_profile = std::move(profile.value);
            Broadcast::profileUpdated();
            this->refreshProfileIfNeeded();
        })
        .catchInMainThread([this]([[maybe_unused]] std::exception&& e) {
            this->_isLoadingProfile = false;
            this->_profile = std::nullopt;
            Broadcast::profileUpdated();
            this->refreshProfileIfNeeded();
        });
}

void CesiumIonSession::refreshAssets() {
    if (!this->_connection || this->_isLoadingAssets) {
        return;
    }

    this->_isLoadingAssets = true;
    this->_loadAssetsQueued = false;

    this->_connection->assets()
        .thenInMainThread([this](Response<Assets>&& assets) {
            this->_isLoadingAssets = false;
            this->_assets = std::move(assets.value);
            Broadcast::assetsUpdated();
            this->refreshAssetsIfNeeded();
        })
        .catchInMainThread([this]([[maybe_unused]] std::exception&& e) {
            this->_isLoadingAssets = false;
            this->_assets = std::nullopt;
            Broadcast::assetsUpdated();
            this->refreshAssetsIfNeeded();
        });
}

void CesiumIonSession::refreshTokens() {
    if (!this->_connection || this->_isLoadingTokens) {
        return;
    }

    this->_isLoadingTokens = true;
    this->_loadTokensQueued = false;

    this->_connection->tokens()
        .thenInMainThread([this](Response<TokenList>&& tokens) {
            this->_isLoadingTokens = false;
            this->_tokens = tokens.value ? std::make_optional(std::move(tokens.value->items)) : std::nullopt;
            Broadcast::tokensUpdated();
            this->refreshTokensIfNeeded();
        })
        .catchInMainThread([this]([[maybe_unused]] std::exception&& e) {
            this->_isLoadingTokens = false;
            this->_tokens = std::nullopt;
            Broadcast::tokensUpdated();
            this->refreshTokensIfNeeded();
        });
}

const std::optional<CesiumIonClient::Connection>& CesiumIonSession::getConnection() const {
    return this->_connection;
}

const CesiumIonClient::Profile& CesiumIonSession::getProfile() {
    static const CesiumIonClient::Profile empty{};
    if (this->_profile) {
        return *this->_profile;
    } else {
        this->refreshProfile();
        return empty;
    }
}

const CesiumIonClient::Assets& CesiumIonSession::getAssets() {
    static const CesiumIonClient::Assets empty;
    if (this->_assets) {
        return *this->_assets;
    } else {
        this->refreshAssets();
        return empty;
    }
}

const std::vector<CesiumIonClient::Token>& CesiumIonSession::getTokens() {
    static const std::vector<CesiumIonClient::Token> empty;
    if (this->_tokens) {
        return *this->_tokens;
    } else {
        this->refreshTokens();
        return empty;
    }
}

bool CesiumIonSession::refreshProfileIfNeeded() {
    if (this->_loadProfileQueued || !this->_profile.has_value()) {
        this->refreshProfile();
    }
    return this->isProfileLoaded();
}

bool CesiumIonSession::refreshAssetsIfNeeded() {
    if (this->_loadAssetsQueued || !this->_assets.has_value()) {
        this->refreshAssets();
    }
    return this->isAssetListLoaded();
}

bool CesiumIonSession::refreshTokensIfNeeded() {
    if (this->_loadTokensQueued || !this->_tokens.has_value()) {
        this->refreshTokens();
    }
    return this->isTokenListLoaded();
}

Future<Response<Token>> CesiumIonSession::findToken(const std::string& token) const {
    if (!this->_connection) {
        return this->getAsyncSystem().createResolvedFuture(
            Response<Token>(0, "NOTCONNECTED", "Not connected to Cesium ion."));
    }

    std::optional<std::string> maybeTokenID = Connection::getIdFromToken(token);

    if (!maybeTokenID) {
        return this->getAsyncSystem().createResolvedFuture(
            Response<Token>(0, "INVALIDTOKEN", "The token is not valid."));
    }

    return this->_connection->token(*maybeTokenID);
}
