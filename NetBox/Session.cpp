/**************************************************************************
 *  NetBox plugin for FAR 2.0 (http://code.google.com/p/farplugs)         *
 *  Copyright (C) 2011 by Artem Senichev <artemsen@gmail.com>             *
 *  Copyright (C) 2011 by Michael Lukashov <michael.lukashov@gmail.com>   *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include "stdafx.h"
#include "Session.h"
#include "Strings.h"
#include "Settings.h"
#include "Protocol.h"
#include "tinyXML\tinyxml.h"

#include "FTP.h"
#include "SFTP.h"
#include "WebDAV.h"

#define OPENSSL_NO_ENGINE
#define OPENSSL_NO_DEPRECATED
#include <openssl/evp.h>
#include <openssl/buffer.h>

string CSession::m_CryptKey = "NetBox";
vector<CSession::ProtoImplInfo> CSession::m_Factory;

static const char *ParamRoot =      "NetBox";
static const char *ParamCrypt =     "crypt";
static const char *ParamProtoId =   "Protocol";
static const char *ParamURL =       "URL";
static const char *ParamUserName =  "UserName";
static const char *ParamPassword =  "Password";
static const char *ParamPromptPsw = "PromptPsw";

static const char *ParamProxyType = "ProxyType";
static const char *ParamProxyHost = "ProxyHost";
static const char *ParamProxyPort = "ProxyPort";
static const char *ParamProxyLogin = "ProxyLogin";
static const char *ParamProxyPassword = "ProxyPassword";

CSession::CSession() : m_ProtoId(-1)
{
    // Инициализируем m_proxySettings значениями по умолчанию
    m_proxySettings.proxyType = m_Settings.ProxyType();
    m_proxySettings.proxyHost = m_Settings.ProxyHost();
    m_proxySettings.proxyPort = m_Settings.ProxyPort();
    m_proxySettings.proxyLogin = m_Settings.ProxyLogin();
    m_proxySettings.proxyPassword = m_Settings.ProxyPassword();
}

void CSession::RegisterProtocolClient(const int id, const wchar_t *name, CreateSessionFx fx, const wchar_t *scheme1, const wchar_t *scheme2 /*= NULL*/)
{
    assert(name);
    assert(fx);
    assert(scheme1);
    assert(GetProtoImplInfo(id) == NULL);

    ProtoImplInfo impl;
    impl.ProtoId = id;
    impl.Name = name;
    impl.Schemes[0] = scheme1;
    impl.Schemes[1] = scheme2;
    impl.CreateSession = fx;
    m_Factory.push_back(impl);
}


const wchar_t *CSession::GetProtocolName(const int protoId)
{
    const ProtoImplInfo *pii = GetProtoImplInfo(protoId);
    assert(pii);
    return pii ? pii->Name : NULL;
}


const wchar_t *CSession::GetDefaultScheme(const int protoId)
{
    const ProtoImplInfo *pii = GetProtoImplInfo(protoId);
    assert(pii);
    return pii ? pii->Schemes[0] : NULL;
}


wstring CSession::GetSupportedPrefixes()
{
    std::wstring prefixes;
    for (vector<ProtoImplInfo>::const_iterator it = m_Factory.begin(); it != m_Factory.end(); ++it)
    {
        if (!prefixes.empty())
        {
            prefixes += L':';
        }
        prefixes += it->Schemes[0];
        if (it->Schemes[1])
        {
            prefixes += L':';
            prefixes += it->Schemes[1];
        }
    }
    return prefixes;
}


PSession CSession::Create()
{
    std::vector<FarMenuItemEx> protos;
    for (vector<ProtoImplInfo>::const_iterator it = m_Factory.begin(); it != m_Factory.end(); ++it)
    {
        FarMenuItemEx item;
        ZeroMemory(&item, sizeof(item));
        item.Text = it->Name;
        item.UserData = it->ProtoId;
        protos.push_back(item);
    }

    const int retCode = CFarPlugin::GetPSI()->Menu(CFarPlugin::GetPSI()->ModuleNumber, -1, -1, 0, FMENU_AUTOHIGHLIGHT | FMENU_WRAPMODE | FMENU_USEEXT, CFarPlugin::GetString(StringEdCrtTitle), NULL, NULL, NULL, NULL, reinterpret_cast<FarMenuItem *>(&protos.front()), static_cast<int>(protos.size()));
    if (retCode < 0)
    {
        return PSession();
    }
    assert(retCode < static_cast<int>(protos.size()));
    const ProtoImplInfo *pii = GetProtoImplInfo(static_cast<int>(protos[retCode].UserData));
    assert(pii);
    PSession session = pii->CreateSession();
    assert(session.get());
    session->m_ProtoId = static_cast<int>(protos[retCode].UserData);
    return (session->Edit() ? session : PSession());
}


PSession CSession::Create(const wchar_t *prefix)
{
    assert(prefix && *prefix);

    std::wstring prefixCmd(prefix);
    const size_t preffixDelim = prefixCmd.find(L':');
    if (preffixDelim == string::npos)
    {
        return PSession();    //wtf?
    }
    if (_wcsnicmp(prefixCmd.c_str(), L"netbox", 6) == 0)
    {
        prefixCmd.erase(0, 6 + 1 /*:*/);
    }

    //Trim first spaces
    while(!prefixCmd.empty() && prefixCmd[0] == L' ')
    {
        prefixCmd.erase(0, 1);
    }

    std::wstring protoScheme(prefixCmd);
    const size_t schemeDelim = protoScheme.find(L':');
    if (schemeDelim == string::npos)
    {
        return PSession();    //no scheme specified
    }
    protoScheme.erase(schemeDelim);

    //Search protocol by scheme
    const ProtoImplInfo *impl = NULL;
    for (vector<ProtoImplInfo>::const_iterator it = m_Factory.begin(); !impl && it != m_Factory.end(); ++it)
    {
        if ((it->Schemes[0] && _wcsicmp(it->Schemes[0], protoScheme.c_str()) == 0) ||
                (it->Schemes[1] && _wcsicmp(it->Schemes[1], protoScheme.c_str()) == 0))
        {
            impl = &(*it);
        }
    }
    if (!impl)
    {
        return PSession();    //Unsupported scheme
    }

    PSession session = impl->CreateSession();
    assert(session.get());
    session->m_ProtoId = impl->ProtoId;
    session->SetURL(prefixCmd.c_str());
    return session;
}


PSession CSession::Create(const int protoId)
{
    const ProtoImplInfo *impl = GetProtoImplInfo(protoId);
    if (!impl)
    {
        return PSession();
    }

    PSession session = impl->CreateSession();
    assert(session.get());
    session->m_ProtoId = protoId;
    return session;
}


PSession CSession::Load(const wchar_t *fileName)
{
    assert(fileName && *fileName);

    CFile xmlFile;
    if (!xmlFile.OpenRead(fileName))
    {
        return PSession();
    }
    size_t buffSize = static_cast<size_t>(xmlFile.GetFileSize() + 1);
    if (buffSize > 1000000)
    {
        return PSession();    //Wrong format
    }
    string buff(buffSize, 0);
    if (!xmlFile.Read(&buff[0], buffSize))
    {
        return PSession();
    }

    //Determine session name from file name
    std::wstring sessionName = fileName;
    const size_t startPos = sessionName.rfind(L'\\');
    assert(startPos != string::npos);
    if (startPos != string::npos)
    {
        sessionName.erase(0, startPos + 1);
    }
    const size_t endPos = sessionName.rfind(L'.');
    assert(endPos != string::npos);
    if (endPos != string::npos)
    {
        sessionName.erase(endPos);
    }

    //Load session
    TiXmlDocument xmlDoc;
    xmlDoc.Parse(buff.c_str());
    if (xmlDoc.Error())
    {
        return PSession();
    }

    //Get and check root node
    const TiXmlElement *xmlRoot = xmlDoc.RootElement();
    if (strcmp(xmlRoot->Value(), ParamRoot) != 0)
    {
        return PSession();
    }

    //Get protocol id and create instance
    const TiXmlElement *xmlProtoId = xmlRoot->FirstChildElement(ParamProtoId);
    if (!xmlProtoId || !xmlProtoId->GetText())
    {
        return PSession();
    }
    PSession session = Create(atoi(xmlProtoId->GetText()));
    if (!session.get())
    {
        return PSession();
    }

    session->SetSessionName(sessionName.c_str());

    //Load session's settings
    const TiXmlNode *xmlNode = NULL;
    while ((xmlNode = xmlRoot->IterateChildren(xmlNode)) != NULL)
    {
        if (strcmp(xmlNode->Value(), ParamProtoId) == 0)
        {
            continue;    //Already read
        }

        const TiXmlElement *xmlEl = xmlNode->ToElement();
        if (!xmlEl)
        {
            continue;
        }

        const char *key = xmlNode->Value();
        const char *plainVal = xmlEl->GetText();

        std::wstring val = plainVal ? ::MB2W(plainVal, CP_UTF8) : std::wstring();
        const char *crypt = xmlEl->Attribute(ParamCrypt);
        if (crypt)
        {
            val = session->Crypt(val, false);
        }

        session->SetProperty(key, val.c_str(), crypt != NULL);
    }

    // Инициализируем m_proxySettings
    struct ProxySettings proxySettings;
    proxySettings.proxyType = static_cast<int>(session->GetPropertyNumeric(ParamProxyType, m_Settings.ProxyType()));
    proxySettings.proxyHost = session->GetProperty(ParamProxyHost, m_Settings.ProxyHost().c_str());
    proxySettings.proxyPort = static_cast<int>(session->GetPropertyNumeric(ParamProxyPort, m_Settings.ProxyPort()));
    proxySettings.proxyLogin = session->GetProperty(ParamProxyLogin, m_Settings.ProxyLogin().c_str());
    proxySettings.proxyPassword = session->GetProperty(ParamProxyPassword, m_Settings.ProxyPassword().c_str());
    session->SetProxySettings(proxySettings);
    return session;
}


PSession CSession::ImportFromFTP(const wchar_t *fileName)
{
    assert(fileName);

    //Get values from ini file
    std::wstring iniUrlVal(256, 0);
    if (!GetPrivateProfileString(L"FarFTP", L"Url", NULL, &iniUrlVal[0], static_cast<DWORD>(iniUrlVal.length()), fileName))
    {
        return PSession();
    }
    std::wstring hostName;
    ParseURL(iniUrlVal.c_str(), NULL, &hostName, NULL, NULL, NULL, NULL, NULL);
    if (hostName.empty())
    {
        return PSession();
    }
    std::wstring iniUsrVal(32, 0);
    GetPrivateProfileString(L"FarFTP", L"User", NULL, &iniUsrVal[0], static_cast<DWORD>(iniUsrVal.length()), fileName);
    std::wstring iniPwdVal(32, 0);
    GetPrivateProfileString(L"FarFTP", L"Password", NULL, &iniPwdVal[0], static_cast<DWORD>(iniPwdVal.length()), fileName);
    INT iniPrPwd = GetPrivateProfileInt(L"FarFTP", L"AskLogin", 0, fileName);
    std::wstring iniCPVal(32, 0);
    GetPrivateProfileString(L"FarFTP", L"CharTable", NULL, &iniCPVal[0], static_cast<DWORD>(iniCPVal.length()), fileName);

    PSession session = PSession(new CSessionFTP());
    session->SetURL(iniUrlVal.c_str());
    session->SetUserName(iniUsrVal.c_str());

    //Decode password
    if (iniPwdVal.size() > 4 && iniPwdVal.substr(0, 4).compare(L"hex:") == 0)
    {
        const string plainHexPwd = ::W2MB(iniPwdVal.substr(4).c_str());
        std::vector<char> unhexPwd;
        for (size_t i = 0; i < plainHexPwd.size(); i += 2)
        {
            int val = 0;
            if (sscanf_s(&plainHexPwd[i], "%02x", &val))
            {
                unhexPwd.push_back(static_cast<char>(val));
            }
        }
        string decodedPwd;
        const char xorMask = (unhexPwd[0] ^ unhexPwd[1]) | 80;
        for (size_t i = 2; i < unhexPwd.size(); ++i)
        {
            const char pwdSymb = unhexPwd[i] ^ xorMask;
            if (pwdSymb == 0 || pwdSymb == xorMask)
            {
                break;
            }
            decodedPwd += pwdSymb;
        }
        session->SetPassword(::MB2W(decodedPwd.c_str()).c_str());
    }
    session->SetPromptPwd(iniPrPwd != 0);

    const UINT codePage = static_cast<UINT>(_wtoi(iniCPVal.c_str()));
    static_cast<CSessionFTP *>(session.get())->SetCodePage(codePage ? codePage : CP_UTF8);

    //Name
    std::wstring sessionName;
    sessionName = L"FarFTP (";
    sessionName += hostName;
    sessionName += L')';
    session->SetSessionName(sessionName.c_str());

    return session;
}


//Export sessions from registry to local folder
// !!!! this functionality will be removed in next release !!!!
void CSession::ExportFromRegistry()
{
    CFarSettings settings;
    if (settings.Open(L"NetBox", true))
    {
        const std::vector<wstring> keys = settings.EnumKeys();
        for (vector<wstring>::const_iterator it = keys.begin(); it != keys.end(); ++it)
        {
            std::wstring regName(L"NetBox");
            regName += L'\\';
            regName += *it;
            if (settings.Open(regName.c_str(), true))
            {
                DWORD regVal = 0;
                if (!settings.GetNumber(L"Proto", regVal))
                {
                    continue;
                }
                const int protoId = static_cast<int>(regVal);
                const ProtoImplInfo *pii = GetProtoImplInfo(protoId);
                if (!pii)
                {
                    continue;
                }
                PSession session = pii->CreateSession();
                assert(session.get());
                session->m_ProtoId = protoId;

                std::wstring sessionName, URL, userName, password, keyFile;
                bool promptPwd = false, useKeyFile = false;
                UINT codePage = CP_UTF8;
                settings.GetString(L"Name", sessionName);
                settings.GetString(L"URL", URL);
                settings.GetString(L"AuthUserName", userName);
                settings.GetString(L"AuthPassword", password);
                password = session->Crypt(password, false);
                if (settings.GetNumber(L"AuthPromptPsw", regVal))
                {
                    promptPwd = (regVal != 0);
                }
                if (settings.GetNumber(L"CodePage", regVal))
                {
                    codePage = regVal;
                }
                if (settings.GetNumber(L"AuthUseKey", regVal))
                {
                    useKeyFile = (regVal != 0);
                }
                if (useKeyFile)
                {
                    settings.GetString(L"AuthKeyFile", keyFile);
                }

                session->SetURL(URL.c_str());
                session->SetUserName(userName.c_str());
                session->SetPassword(password.c_str());
                session->SetPromptPwd(promptPwd);

                switch (protoId)
                {
                case 0:
                    static_cast<CSessionFTP *>(session.get())->SetCodePage(codePage);
                    break;
                case 1:
                    static_cast<CSessionSFTP *>(session.get())->SetCodePage(codePage);
                    static_cast<CSessionSFTP *>(session.get())->SetKeyFile(keyFile.c_str());
                    break;
                }

                size_t pos = string::npos;
                while ((pos = sessionName.find_first_of(L"<>:\"/\\|?*")) != string::npos)
                {
                    sessionName[pos] = L'_';
                }
                session->SetSessionName(sessionName.c_str());

                std::wstring savePath = m_Settings.GetSessionPath();
                CreateDirectory(savePath.c_str(), NULL);

                if (session->Save(savePath.c_str(), false))
                {
                    settings.DeleteTree(regName.c_str());
                }
            }
        }
    }
}



bool CSession::Edit()
{
    PSessionEditor editor = CreateEditorInstance();
    assert(editor.get());
    return editor->EditSession();
}


bool CSession::Save(const wchar_t *fileName) const
{
    assert(fileName && *fileName);

    TiXmlDocument xmlDoc;
    xmlDoc.LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", ""));
    TiXmlElement *xmlRoot = new TiXmlElement(ParamRoot);
    xmlDoc.LinkEndChild(xmlRoot);

    TiXmlElement *xmlNode;
    xmlNode = new TiXmlElement(ParamProtoId);
    xmlNode->LinkEndChild(new TiXmlText(NumberToText(m_ProtoId).c_str()));
    xmlRoot->LinkEndChild(xmlNode);

    for (vector<Property>::const_iterator it = m_Properties.begin(); it != m_Properties.end(); ++it)
    {
        xmlNode = new TiXmlElement(it->Name.c_str());
        const string val = ::W2MB(it->NeedCrypt ? Crypt(it->Value, true).c_str() : it->Value.c_str(), CP_UTF8);
        xmlNode->LinkEndChild(new TiXmlText(val.c_str()));
        if (it->NeedCrypt)
        {
            xmlNode->SetAttribute(ParamCrypt, 1);
        }
        xmlRoot->LinkEndChild(xmlNode);
    }

    TiXmlPrinter xmlPrinter;
    xmlPrinter.SetIndent("\t");
    xmlPrinter.SetLineBreak("\r\n");
    xmlDoc.Accept(&xmlPrinter);
    const char *xmlContent = xmlPrinter.CStr();
    if (!xmlContent || !*xmlContent)
    {
        return false;
    }

    return (CFile::SaveFile(fileName, xmlContent) == ERROR_SUCCESS);
}


bool CSession::Save(const wchar_t *path, const bool overwrite) const
{
    assert(path && *path);

    std::wstring savePath = path;
    ::AppendPathDelimiterW(savePath);

    std::wstring fileName = savePath;
    fileName += GetSessionName();
    fileName += L".netbox";

    if (!overwrite)
    {
        const size_t maxPathLength = savePath.length() + wcslen(GetSessionName()) + 64;
        int counter = 1;
        while (GetFileAttributes(fileName.c_str()) != INVALID_FILE_ATTRIBUTES)
        {
            fileName.resize(maxPathLength, 0);
            fileName.resize(swprintf_s(&fileName[0], fileName.length(), L"%s%s (%d).netbox", savePath.c_str(), GetSessionName(), ++counter));
        }
    }

    return Save(fileName.c_str());
}


PProtocol CSession::CreateClient()
{
    //Check url
    std::wstring hostName, userName, password;
    ParseURL(GetURL(), NULL, &hostName, NULL, NULL, NULL, &userName, &password);
    if (hostName.empty())
    {
        CFarPlugin::MessageBox(CFarPlugin::GetString(StringTitle), CFarPlugin::GetString(StringEdErrURLInvalid), FMSG_MB_OK | FMSG_WARNING);
        return PProtocol();
    }

    if (!userName.empty() || !password.empty())
    {
        SetUserName(userName.c_str());
        SetPassword(password.c_str());
    }

    if (GetPromptPwd())
    {
        CFarDialog dlg(40, 10, CFarPlugin::GetString(StringEdAuth));
        dlg.CreateText(dlg.GetLeft(), dlg.GetTop() + 0, CFarPlugin::GetString(StringEdAuthUser));
        const int idName = dlg.CreateEdit(dlg.GetLeft(), dlg.GetTop() + 1, MAX_SIZE, GetUserName());
        dlg.CreateText(dlg.GetLeft(), dlg.GetTop() + 2, CFarPlugin::GetString(StringEdAuthPsw));
        FarDialogItem *pwdItem;
        const int idPwd = dlg.CreateDlgItem(DI_PSWEDIT, dlg.GetLeft(), dlg.GetWidth(), dlg.GetTop() + 3, dlg.GetTop() + 3, GetPassword(), 0, &pwdItem);
        if (wcslen(GetUserName()) != 0)
        {
            pwdItem->Focus = 1;
        }
        dlg.CreateSeparator(dlg.GetHeight() - 2);
        dlg.CreateButton(0, dlg.GetHeight() - 1, CFarPlugin::GetString(StringOK), DIF_CENTERGROUP);
        const int idBtnCancel = dlg.CreateButton(0, dlg.GetHeight() - 1, CFarPlugin::GetString(StringCancel), DIF_CENTERGROUP);
        const int retCode = dlg.DoModal();
        if (retCode < 0 || retCode == idBtnCancel)
        {
            return PProtocol();
        }
        else
        {
            SetUserName(dlg.GetText(idName).c_str());
            SetPassword(dlg.GetText(idPwd).c_str());
        }
    }

    return CreateClientInstance();
}


const wchar_t *CSession::GetSessionName() const
{
    return m_SessionName.c_str();
}


const wchar_t *CSession::GetURL() const
{
    return GetProperty(ParamURL, L"");
}


const wchar_t *CSession::GetUserName() const
{
    return GetProperty(ParamUserName, L"");
}


const wchar_t *CSession::GetPassword() const
{
    return GetProperty(ParamPassword, L"");
}


void CSession::SetSessionName(const wchar_t *val)
{
    assert(val);
    m_SessionName = val;
}


void CSession::SetURL(const wchar_t *val)
{
    assert(val);
    SetProperty(ParamURL, val);
}


void CSession::SetUserName(const wchar_t *val)
{
    assert(val);
    SetProperty(ParamUserName, val);
}


void CSession::SetPassword(const wchar_t *val)
{
    assert(val);
    SetProperty(ParamPassword, val, true);
}


bool CSession::GetPromptPwd() const
{
    return (GetPropertyNumeric(ParamPromptPsw) != 0);
}


void CSession::SetPromptPwd(const bool val)
{
    SetProperty(ParamPromptPsw, val ? 1 : 0);
}

void CSession::SetProxySettings(const struct ProxySettings &proxySettings)
{
    // SetProperty(ParamPromptPsw, val ? 1 : 0);
    m_proxySettings = proxySettings;
    SetProperty(ParamProxyType, m_proxySettings.proxyType);
    SetProperty(ParamProxyHost, m_proxySettings.proxyHost.c_str());
    SetProperty(ParamProxyPort, m_proxySettings.proxyPort);
    SetProperty(ParamProxyLogin, m_proxySettings.proxyLogin.c_str());
    SetProperty(ParamProxyPassword, m_proxySettings.proxyPassword.c_str(), true);
}

const wchar_t *CSession::GetProperty(const char *name, const wchar_t *defaultVal /*= NULL*/) const
{
    assert(name);

    for (vector<Property>::const_iterator it = m_Properties.begin(); it != m_Properties.end(); ++it)
    {
        if (strcmp(name, it->Name.c_str()) == 0)
        {
            return it->Value.c_str();
        }
    }
    return defaultVal;
}


__int64 CSession::GetPropertyNumeric(const char *name, const __int64 defaultVal /*= 0*/) const
{
    assert(name);
    const wchar_t *val = GetProperty(name);
    // DEBUG_PRINTF(L"NetBox: GetPropertyNumeric: val = %s", val);
    return (val ? _wtoi64(val) : defaultVal);
}


void CSession::SetProperty(const char *name, const wchar_t *val, const bool needCrypt /*= false*/)
{
    assert(name);
    assert(val);

    Property *prop = NULL;
    for (vector<Property>::iterator it = m_Properties.begin(); !prop && it != m_Properties.end(); ++it)
    {
        if (strcmp(name, it->Name.c_str()) == 0)
        {
            prop = &(*it);
        }
    }
    Property propNew;
    bool createNew = (prop == NULL);
    if (createNew)
    {
        prop = &propNew;
        prop->Name = name;
    }
    prop->Value = val;
    prop->NeedCrypt = needCrypt;
    if (createNew)
    {
        m_Properties.push_back(*prop);
    }
}


void CSession::SetProperty(const char *name, const __int64 val)
{
    assert(name);
    wchar_t numTxt[32];
    _i64tow_s(val, numTxt, sizeof(numTxt) / sizeof(wchar_t), 10);
    SetProperty(name, numTxt);
}


wstring CSession::Crypt(const std::wstring &src, const bool encrypt) const
{
    if (src.empty())
    {
        return std::wstring();
    }

    if (m_Settings.UseOwnKey())
    {
        static bool sessionPasswordPromted = false;
        if (!sessionPasswordPromted)
        {
            wchar_t pwd[256];
            if (CFarPlugin::GetPSI()->InputBox(CFarPlugin::GetString(StringTitle), CFarPlugin::GetString(StringSessionPwd), NULL, NULL, pwd, sizeof(pwd) / sizeof(wchar_t), NULL, FIB_ENABLEEMPTY | FIB_PASSWORD))
            {
                m_CryptKey = ::W2MB(pwd);
                sessionPasswordPromted = true;
            }
        }
    }

    const EVP_CIPHER *cipher = EVP_aes_256_cbc();

    EVP_CIPHER_CTX ctx;

    unsigned char key[32], iv[32];
    EVP_BytesToKey(cipher, EVP_sha1(), NULL, reinterpret_cast<const unsigned char *>(m_CryptKey.c_str()), static_cast<int>(m_CryptKey.length()), 5, key, iv);

    EVP_CIPHER_CTX_init(&ctx);
    BIO *b64 = BIO_new(BIO_f_base64());

    std::wstring encodedVal;

    if (encrypt)
    {
        //Encrypt
        int encryptBufferLen = static_cast<int>(src.length() * sizeof(wchar_t) + 256);
        std::vector<unsigned char> encryptBuffer(static_cast<size_t>(encryptBufferLen));
        int finalLen;
        if (EVP_EncryptInit_ex(&ctx, cipher, NULL, key, iv) &&
                EVP_EncryptUpdate(&ctx, &encryptBuffer.front(), &encryptBufferLen, reinterpret_cast<const unsigned char *>(src.c_str()), static_cast<int>(src.length() * sizeof(wchar_t))) &&
                EVP_EncryptFinal_ex(&ctx, &encryptBuffer[encryptBufferLen], &finalLen))
        {
            encryptBufferLen += finalLen;
            //Base64 encode
            BIO *bmem = BIO_new(BIO_s_mem());
            b64 = BIO_push(b64, bmem);
            BIO_write(b64, &encryptBuffer.front(), encryptBufferLen);
            BIO_flush(b64);
            BUF_MEM *bptr;
            if (BIO_get_mem_ptr(b64, &bptr))
            {
                encodedVal = ::MB2W(string(static_cast<const char *>(bptr->data), bptr->length).c_str());
            }
        }
    }
    else
    {
        //Base64 decode
        string base64Value = ::W2MB(src.c_str());
        BIO *bmem = BIO_new_mem_buf(&base64Value[0], static_cast<int>(base64Value.length()));
        bmem = BIO_push(b64, bmem);

        std::vector<unsigned char> decryptBuffer(base64Value.length());
        const int decryptBufferLen = BIO_read(bmem, &decryptBuffer.front(), static_cast<int>(decryptBuffer.size()));
        if (decryptBufferLen)
        {
            encodedVal.resize(decryptBufferLen);
            int decodeLen = decryptBufferLen * sizeof(wchar_t);
            int finalLen;
            //Decrypt
            if (EVP_DecryptInit_ex(&ctx, cipher, NULL, key, iv) &&
                    EVP_DecryptUpdate(&ctx, reinterpret_cast<unsigned char *>(&encodedVal[0]), &decodeLen, &decryptBuffer.front(), decryptBufferLen) &&
                    EVP_DecryptFinal_ex(&ctx, reinterpret_cast<unsigned char *>(&encodedVal[decodeLen]), &finalLen))
            {
                decodeLen += finalLen;
                encodedVal.resize(decodeLen / sizeof(wchar_t));
            }
        }
    }

    BIO_free_all(b64);
    EVP_CIPHER_CTX_cleanup(&ctx);

    return encodedVal;
}


const CSession::ProtoImplInfo *CSession::GetProtoImplInfo(const int protoId)
{
    const ProtoImplInfo *res = NULL;
    for (vector<ProtoImplInfo>::const_iterator it = m_Factory.begin(); !res && it != m_Factory.end(); ++it)
    {
        if (protoId == it->ProtoId)
        {
            res = &(*it);
        }
    }
    return res;
}
