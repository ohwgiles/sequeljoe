#ifndef SAVEDCONFIG_H
#define SAVEDCONFIG_H

struct SavedConfig {
    static constexpr int DEFAULT_SQL_PORT = 3306;
    static constexpr int DEFAULT_SSH_PORT = 22;



    static constexpr const char* KEY_HOST = "Host";
    static constexpr const char* KEY_DBNM = "DbName";
    static constexpr const char* KEY_PORT = "Port";
    static constexpr const char* KEY_TYPE = "Type";
    static constexpr const char* KEY_USER = "Username";
    static constexpr const char* KEY_PASS = "Password";

    static constexpr const char* KEY_USE_SSH = "UseSsh";
    static constexpr const char* KEY_SSH_HOST = "SshHost";
    static constexpr const char* KEY_SSH_PORT = "SshPort";
    static constexpr const char* KEY_SSH_USER = "SshUser";
    static constexpr const char* KEY_SSH_PASS = "SshPass";
    static constexpr const char* KEY_SSH_KEY = "SshKeyPath";

};

#endif // SAVEDCONFIG_H
