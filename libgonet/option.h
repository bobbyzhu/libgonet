#pragma once
#include "config.h"
#include "abstract.h"

namespace network {

struct OptionSSL
{
#if ENABLE_SSL
    enum class verify_mode_t
    {
        none,
        optional,
        required,
    };

    enum class ssl_version_t
    {
        sslv2 = ::boost::asio::ssl::context::sslv2,
        sslv3 = ::boost::asio::ssl::context::sslv3,
        tlsv1 = ::boost::asio::ssl::context::tlsv1,
        sslv23 = ::boost::asio::ssl::context::sslv23,
        tlsv11 = ::boost::asio::ssl::context::tlsv11,
        tlsv12 = ::boost::asio::ssl::context::tlsv12,
    };

    ssl_version_t ssl_version = ssl_version_t::sslv23;
    verify_mode_t verify_mode = verify_mode_t::none;
    bool disable_compression = false;
    boost::function<std::string(std::size_t, int)> pwd_callback;
    std::string certificate_chain_file;
    std::string private_key_file;
    std::string tmp_dh_file;
    std::string verify_file;
#endif
};

struct OptionsUser
{
    int listen_backlog_ = ::boost::asio::ip::tcp::socket::max_connections;
    int sndtimeo_ = 0;
    uint32_t max_pack_size_ = 64 * 1024;
    uint32_t max_connection_ = std::numeric_limits<uint32_t>::max();
    OptionSSL ssl_option_;
};

struct OptionsData : public OptionsUser
{
    ConnectedCb connect_cb_;
    ReceiveCb receive_cb_;
    DisconnectedCb disconnect_cb_;

    static OptionsData& DefaultOption()
    {
        static OptionsData data;
        return data;
    }
};

struct OptionsBase
{
    OptionsData opt_;
    std::vector<OptionsBase*> lnks_;
    OptionsBase* parent_ = nullptr;

    virtual ~OptionsBase()
    {
        if (parent_)
            parent_->Unlink(this);

        while (!lnks_.empty())
            Unlink(lnks_.back());
    }

    void Link(OptionsBase & other)
    {
        other.opt_ = this->opt_;
        lnks_.push_back(&other);
        assert(nullptr == other.parent_);
        other.parent_ = this;
        other.OnLink();
    }

    void Unlink(OptionsBase * other)
    {
        assert(other->parent_ == this);
        other->parent_ = nullptr;
        auto it = std::find(lnks_.begin(), lnks_.end(), other);
        if (lnks_.end() != it)
            lnks_.erase(it);
    }

    void OnLink()
    {
        for (auto o:lnks_) {
            o->opt_ = this->opt_;
            o->OnLink();
        }
    }

    void SetConnectedCb(ConnectedCb cb)
    {
        opt_.connect_cb_ = cb;
        OnSetConnectedCb();
        for (auto o:lnks_)
            o->SetConnectedCb(cb);
    }
    void SetReceiveCb(ReceiveCb cb)
    {
        opt_.receive_cb_ = cb;
        OnSetReceiveCb();
        for (auto o:lnks_)
            o->SetReceiveCb(cb);
    }
    void SetDisconnectedCb(DisconnectedCb cb)
    {
        opt_.disconnect_cb_ = cb;
        OnSetDisconnectedCb();
        for (auto o:lnks_)
            o->SetDisconnectedCb(cb);
    }
    void SetListenBacklog(int listen_backlog)
    {
        opt_.listen_backlog_ = listen_backlog;
        OnSetListenBacklog();
        for (auto o:lnks_)
            o->SetListenBacklog(listen_backlog);
    }
    void SetSndTimeout(int sndtimeo)
    {
        opt_.sndtimeo_ = sndtimeo;
        OnSetSndTimeout();
        for (auto o:lnks_)
            o->SetSndTimeout(sndtimeo);
    }
    void SetMaxPackSize(uint32_t max_pack_size)
    {
        opt_.max_pack_size_ = max_pack_size;
        OnSetMaxPackSize();
        for (auto o:lnks_)
            o->SetMaxPackSize(max_pack_size);
    }
    void SetMaxConnection(uint32_t max_connection)
    {
        opt_.max_connection_ = max_connection;
        OnSetMaxConnection();
        for (auto o:lnks_)
            o->SetMaxConnection(max_connection);
    }
    void SetSSLOption(OptionSSL const& opt)
    {
        opt_.ssl_option_ = opt;
        OnSetSSLOption();
        for (auto o:lnks_)
            o->SetSSLOption(opt);
    }
    virtual void OnSetConnectedCb() {}
    virtual void OnSetReceiveCb() {}
    virtual void OnSetDisconnectedCb() {}
    virtual void OnSetListenBacklog() {}
    virtual void OnSetSndTimeout() {}
    virtual void OnSetMaxPackSize() {}
    virtual void OnSetMaxConnection() {}
    virtual void OnSetSSLOption() {}
};

template <typename Drived>
struct Options : public OptionsBase
{
    Drived& GetThisDrived()
    {
        return *static_cast<Drived*>(this);
    }

    Drived& SetConnectedCb(ConnectedCb cb)
    {
        OptionsBase::SetConnectedCb(cb);
        return GetThisDrived();
    }
    Drived& SetReceiveCb(ReceiveCb cb)
    {
        OptionsBase::SetReceiveCb(cb);
        return GetThisDrived();
    }
    Drived& SetDisconnectedCb(DisconnectedCb cb)
    {
        OptionsBase::SetDisconnectedCb(cb);
        return GetThisDrived();
    }
    Drived& SetListenBacklog(int listen_backlog)
    {
        OptionsBase::SetListenBacklog(listen_backlog);
        return GetThisDrived();
    }
    Drived& SetSndTimeout(int sndtimeo)
    {
        OptionsBase::SetSndTimeout(sndtimeo);
        return GetThisDrived();
    }
    Drived& SetMaxPackSize(uint32_t max_pack_size)
    {
        OptionsBase::SetMaxPackSize(max_pack_size);
        return GetThisDrived();
    }
    Drived& SetMaxConnection(uint32_t max_connection)
    {
        OptionsBase::SetMaxConnection(max_connection);
        return GetThisDrived();
    }
    Drived& SetSSLOption(OptionSSL const& opt)
    {
        OptionsBase::SetSSLOption(opt);
        return GetThisDrived();
    }
};

}//namespace network
