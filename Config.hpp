#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <unistd.h>

// MYSQL parameters
static const std::string URL = "tcp://127.0.0.1:3306";
static const std::string USER = "root";
static const std::string PASSWORD = "if you have a password put it here";

// OANDA parameters
static const std::string ACCOUNT_ID = "<Put id here>";
static const std::string ACCESS_TOKEN = "<Put token here>";

#endif
