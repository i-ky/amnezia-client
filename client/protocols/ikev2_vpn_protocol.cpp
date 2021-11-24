#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>
//#include <QRegularExpression>
//#include <QTcpSocket>
#include <QThread>

#include <chrono>

#include "debug.h"
#include "ikev2_vpn_protocol.h"
#include "utils.h"

static Ikev2Protocol* self = nullptr;
static std::mutex rasDialFuncMutex;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern "C" {
static void WINAPI RasDialFuncCallback(UINT unMsg,
                                       RASCONNSTATE rasconnstate,
                                       DWORD dwError );
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Ikev2Protocol::Ikev2Protocol(const QJsonObject &configuration, QObject* parent) :
    VpnProtocol(configuration, parent)
{
    self = this;
    //m_configFile.setFileTemplate(QDir::tempPath() + QDir::separator() + serviceName() + ".conf");
    readIkev2Configuration(configuration);
}

Ikev2Protocol::~Ikev2Protocol()
{
    qDebug() << "IpsecProtocol::~IpsecProtocol()";
#ifdef Q_OS_WIN
    disconnect_vpn();
#endif
    Ikev2Protocol::stop();
}

void Ikev2Protocol::stop()
{
    setConnectionState(VpnProtocol::Disconnecting);
#ifdef Q_OS_WINDOWS
    {
        if (! disconnect_vpn() ){
            qDebug()<<"We don't disconnect";
            setConnectionState(VpnProtocol::Error);
        }
        else {
            setConnectionState(VpnProtocol::Disconnected);
        }
    }
#endif
}

void Ikev2Protocol::newConnectionStateEventReceived(UINT unMsg, tagRASCONNSTATE rasconnstate, DWORD dwError)
{
    Q_UNUSED(unMsg);
    qDebug()<<"Recive the new event "<<static_cast<int>(rasconnstate);
    switch (rasconnstate)
    {
    case RASCS_OpenPort:
        qDebug()<<__FUNCTION__ << __LINE__;
        setConnectionState(Preparing);
        break;
    case RASCS_PortOpened:
        qDebug()<<__FUNCTION__ << __LINE__;
        setConnectionState(Preparing);
        break;
    case RASCS_ConnectDevice:
        qDebug()<<__FUNCTION__ << __LINE__;
        setConnectionState(Preparing);
        break;
    case RASCS_DeviceConnected:
        qDebug()<<__FUNCTION__ << __LINE__;
        setConnectionState(Preparing);
        break;
    case RASCS_AllDevicesConnected:
        qDebug()<<__FUNCTION__ << __LINE__;
        setConnectionState(Preparing);
        break;
    case RASCS_Authenticate:
        qDebug()<<__FUNCTION__ << __LINE__;
        setConnectionState(Preparing);
        break;
    case RASCS_AuthNotify:
        qDebug()<<__FUNCTION__ << __LINE__;
        if (dwError != 0) {
            qDebug() << "have error" << dwError;
            setConnectionState(Disconnected);
        } else {
            qDebug() << "RASCS_AuthNotify but no error" << dwError;
        }
        break;
    case RASCS_AuthRetry:
        qDebug()<<__FUNCTION__ << __LINE__;
        setConnectionState(Preparing);
        break;
    case RASCS_AuthCallback:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_AuthChangePassword:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_AuthProject:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_AuthLinkSpeed:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_AuthAck:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_ReAuthenticate:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_Authenticated:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_PrepareForCallback:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_WaitForModemReset:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_WaitForCallback:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_Projected:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
#if (WINVER >= 0x400)
    case RASCS_StartAuthentication:    // Windows 95 only
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_CallbackComplete:       // Windows 95 only
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_LogonNetwork:           // Windows 95 only
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
#endif
    case RASCS_SubEntryConnected:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_SubEntryDisconnected:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
        //PAUSED STATES:
    case RASCS_Interactive:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_RetryAuthentication:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_CallbackSetByCaller:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    case RASCS_PasswordExpired:
        setConnectionState(Error);
        qDebug()<<__FUNCTION__ << __LINE__;
        break;

    case RASCS_Connected: // = RASCS_DONE:
        setConnectionState(Connected);
        qDebug()<<__FUNCTION__ << __LINE__;
        //printf ("RASCS_Connected = %d\n", _connection_state);
        //printf ("Connection completed.\n");
        //SetEvent(gEvent_handle);
    {
        //get the network settings of adapters
        std::string p1,p2,p3;
        const auto ret = adpInfo.get_adapter_info(tunnelName().toStdString());
        if (std::get<0>(ret) == false){
            p1 = adpInfo.get_adapter_route_gateway();
            p2 = adpInfo.get_adapter_local_address();
            p3 = adpInfo.get_adapter_local_gateway();
            m_routeGateway = QString::fromStdString(p1);
            m_vpnLocalAddress = QString::fromStdString(p2);
            m_vpnGateway = QString::fromStdString(p3);
            qDebug()<<"My ikev2 m_routeGateway "<<m_routeGateway;
            qDebug()<<"My ikev2 m_vpnLocalAddress "<<m_vpnLocalAddress;
            qDebug()<<"My ikev2 m_vpnGateway "<< m_vpnGateway;
        }
    }
        break;
    case RASCS_Disconnected:
        setConnectionState(Disconnected);
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    default:
        qDebug()<<__FUNCTION__ << __LINE__;
        break;
    }
}

void Ikev2Protocol::readIkev2Configuration(const QJsonObject &configuration)
{
    m_config = configuration.value(ProtocolProps::key_proto_config_data(Protocol::Ikev2)).toObject();
}

ErrorCode Ikev2Protocol::start()
{
#ifdef Q_OS_WINDOWS
    QByteArray cert = QByteArray::fromBase64(m_config[config_key::cert].toString().toUtf8());
    setConnectionState(Connecting);

    QTemporaryFile certFile;
    certFile.setAutoRemove(false);
    certFile.open();
    certFile.write(cert);
    certFile.close();

    {
        auto certInstallProcess = IpcClient::CreatePrivilegedProcess();

        if (!certInstallProcess) {
            setLastError(ErrorCode::AmneziaServiceConnectionFailed);
            return ErrorCode::AmneziaServiceConnectionFailed;
        }

        certInstallProcess->waitForSource(1000);
        if (!certInstallProcess->isInitialized()) {
            qWarning() << "IpcProcess replica is not connected!";
            setLastError(ErrorCode::AmneziaServiceConnectionFailed);
            return ErrorCode::AmneziaServiceConnectionFailed;
        }
        certInstallProcess->setProgram("certutil");
        QStringList arguments({"-f" , "-p", m_config[config_key::password].toString(),
                               "-importpfx", certFile.fileName(), "NoExport"
                              });
        certInstallProcess->setArguments(arguments);

//        qDebug() << arguments.join(" ");
//        connect(certInstallProcess.data(), &PrivilegedProcess::errorOccurred, [certInstallProcess](QProcess::ProcessError error) {
//            qDebug() << "PrivilegedProcess errorOccurred" << error;
//        });

//        connect(certInstallProcess.data(), &PrivilegedProcess::stateChanged, [certInstallProcess](QProcess::ProcessState newState) {
//            qDebug() << "PrivilegedProcess stateChanged" << newState;
//        });

//        connect(certInstallProcess.data(), &PrivilegedProcess::readyRead, [certInstallProcess]() {
//            auto req = certInstallProcess->readAll();
//            req.waitForFinished();
//            qDebug() << "PrivilegedProcess readyRead" << req.returnValue();
//        });


        certInstallProcess->start();
    }
    ///*
    {
        //        auto adapterRemoveProcess = new QProcess;

        //        adapterRemoveProcess->setProgram("powershell");
        //        QString arguments = QString("-command \"Remove-VpnConnection -Name '%1' -Force\"").arg(tunnelName());
        //        adapterRemoveProcess->setNativeArguments(arguments);

        //        adapterRemoveProcess->start();
        //        adapterRemoveProcess->waitForFinished(5000);
        if ( disconnect_vpn()){
            qDebug()<<"VPN was disconnected";
        }
        if ( delete_vpn_connection (tunnelName())){
            qDebug()<<"VPN was deleted";
        }
    }

    {
        {
            if ( !create_new_vpn(tunnelName(), m_config[config_key::hostName].toString())){
                qDebug() <<"Can't create the VPN connect";
            }
        }
        //        auto adapterInstallProcess = new QProcess;

        //        adapterInstallProcess->setProgram("powershell");
        //        QString arguments = QString("-command \"Add-VpnConnection "
        //                                    "-ServerAddress '%1' "
        //                                    "-Name '%2' "
        //                                    "-TunnelType IKEv2 "
        //                                    "-AuthenticationMethod MachineCertificate "
        //                                    "-EncryptionLevel Required "
        //                                    "-PassThru\"")
        //                .arg(m_config[config_key::hostName].toString())
        //                .arg(tunnelName());
        //        adapterInstallProcess->setNativeArguments(arguments);
        //        adapterInstallProcess->start();
        //        adapterInstallProcess->waitForFinished(5000);
    }

    {
        auto adapterConfigProcess = new QProcess;

        adapterConfigProcess->setProgram("powershell");
        QString arguments = QString("-command \"Set-VpnConnectionIPsecConfiguration\" "
                                    "-ConnectionName '%1' "
                                    "-AuthenticationTransformConstants GCMAES128 "
                                    "-CipherTransformConstants GCMAES128 "
                                    "-EncryptionMethod AES256 "
                                    "-IntegrityCheckMethod SHA256 "
                                    "-PfsGroup None "
                                    "-DHGroup Group14 "
                                    "-PassThru -Force\"")
                .arg(tunnelName());
        adapterConfigProcess->setNativeArguments(arguments);

        //        connect(adapterConfigProcess, &QProcess::readyRead, [adapterConfigProcess]() {
        //            qDebug().noquote() << "adapterConfigProcess readyRead" << adapterConfigProcess->readAll();
        //        });

        adapterConfigProcess->start();
        adapterConfigProcess->waitForFinished(5000);
    }
    //*/
    {
        if (!connect_to_vpn(tunnelName())) {
            qDebug()<<"We can't connect to VPN";
        }
    }
    //setConnectionState(Connecting);
    return ErrorCode::NoError;
#else
    return ErrorCode::NoError;
#endif

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef Q_OS_WINDOWS
bool Ikev2Protocol::create_new_vpn(const QString & vpn_name,
                                   const QString & serv_addr){

    if ( RasValidateEntryName(nullptr, vpn_name.toStdWString().c_str()) != ERROR_SUCCESS)
        return false;
    DWORD size = 0;
    ::RasGetEntryProperties(nullptr, L"", nullptr, &size, nullptr, nullptr);
    LPRASENTRY pras = static_cast<LPRASENTRY>(malloc(size));
    memset(pras, 0, size);
    pras->dwSize = size;
    pras->dwType = RASET_Vpn;
    pras->dwRedialCount = 1;
    pras->dwRedialPause = 60;
    pras->dwfNetProtocols =  RASNP_Ip|RASNP_Ipv6;
    pras->dwEncryptionType = ET_RequireMax;
    wcscpy_s(pras->szLocalPhoneNumber, serv_addr.toStdWString().c_str());
    wcscpy_s(pras->szDeviceType, RASDT_Vpn);
    pras->dwfOptions = RASEO_RemoteDefaultGateway;
    pras->dwfOptions |= RASEO_RequireDataEncryption;
    pras->dwfOptions2 |= RASEO2_RequireMachineCertificates;
    pras->dwVpnStrategy = VS_Ikev2Only;
    const auto nRet = ::RasSetEntryProperties(nullptr, vpn_name.toStdWString().c_str(), pras, pras->dwSize, NULL, 0);
    free(pras);
    if (nRet == ERROR_SUCCESS)
        return true;
    return false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool Ikev2Protocol::delete_vpn_connection(const QString &vpn_name){

    if ( RasDeleteEntry(nullptr, vpn_name.toStdWString().c_str()) == ERROR_SUCCESS){
        return true;
    }
    return false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool Ikev2Protocol::connect_to_vpn(const QString & vpn_name){
    RASDIALPARAMS RasDialParams;
    memset(&RasDialParams, 0x0, sizeof(RASDIALPARAMS));
    RasDialParams.dwSize = sizeof(RASDIALPARAMS);
    wcscpy_s(RasDialParams.szEntryName, vpn_name.toStdWString().c_str());
    auto ret = RasDial(NULL, NULL, &RasDialParams, 0,
                       &RasDialFuncCallback,
                       &hRasConn);
    if (ret == ERROR_SUCCESS){
        return true;
    }
    return false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool Ikev2Protocol::disconnect_vpn(){
    if ( hRasConn != nullptr ){
        if ( RasHangUp(hRasConn) != ERROR_SUCCESS)
            return false;
    }
    QThread::msleep(3000);

    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void WINAPI RasDialFuncCallback(UINT unMsg,
                                RASCONNSTATE rasconnstate,
                                DWORD dwError ){
    std::lock_guard<std::mutex> guard(rasDialFuncMutex);
    if (self) {
        self->newConnectionStateEventReceived(unMsg, rasconnstate, dwError);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//void get_connecting_status(std::string_view _vpn_name,
//                           std::string &_m_route_gateway,
//                           std::string & _m_vpn_local_address,
//                           std::string &_m_vpn_gateway){
//    if (_vpn_name.length() == 0)
//        return;
//    IP_ADAPTER_INFO *adapterInfo{nullptr};
//    DWORD adapLen = sizeof(IP_ADAPTER_INFO);
//    adapterInfo = (IP_ADAPTER_INFO*)malloc(adapLen);
//    if (adapterInfo == nullptr)
//        return;
//    auto status{::GetAdaptersInfo(adapterInfo, &adapLen)};
//    if (status == ERROR_BUFFER_OVERFLOW){
//        free (adapterInfo);
//        adapterInfo = (IP_ADAPTER_INFO*)malloc(adapLen);
//        status = ::GetAdaptersInfo(adapterInfo, &adapLen);
//        if (adapterInfo == nullptr)
//            return ;
//    }
//    auto it = adapterInfo;
//    while (it){
//        if (strcmp(_vpn_name.data(), it->Description) == 0){
//            //
//            adpinfo::NetAdpInfo ni;
//            auto tmp_str  = ni.get_system_route();
//            _m_route_gateway = tmp_str;
//            //memcpy(_m_route_gateway.data(), rsg.c_str(), rsg.length());
//            _m_vpn_local_address = it->IpAddressList.IpAddress.String;
//            //memcpy(_m_vpn_local_address.data(), it->IpAddressList.IpAddress.String, 16);
//            _m_vpn_gateway =  it->GatewayList.IpAddress.String;
//            //memcpy(_m_vpn_gateway.data(), it->GatewayList.IpAddress.String,16);
//            qDebug()<<"FINDED";
//            break;
//        }
//        it = it->Next;
//    }
//    free(adapterInfo);
//}
#endif
