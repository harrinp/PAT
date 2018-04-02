//=================================================================================================
//                    Copyright (C) 2017 Olivier Mallet - All Rights Reserved
//=================================================================================================

#ifndef QUOTESDB_HPP
#define QUOTESDB_HPP

//=================================================================================================

#include <iomanip>

// POCO headers
#include <Poco/Net/HTTPSClientSession.h> // for HTTPSClientSession
#include <Poco/Net/HTTPRequest.h>        // for HTTPRequest
#include <Poco/Net/HTTPResponse.h>       // for HTTPResponse
#include <Poco/Net/SSLManager.h>         // for Context
#include <Poco/URI.h>                    // for URI
#include <Poco/Exception.h>              // for try/catch and Exception
#include <Poco/JSON/Array.h>             // for Array
#include <Poco/JSON/Object.h>            // for Object
#include <Poco/JSON/Parser.h>            // for Parser
#include <Poco/Dynamic/Var.h>            // for Var

// MYSQL headers
#include <mysql_connection.h>
#include <mysql/mysql.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

/*-------------------------------------------------------------------------------------------------*/

// MYSQL parameters
static const std::string URL = "tcp://127.0.0.1:3306";
static const std::string USER = "root";
static const std::string PASSWORD = "";

// OANDA parameters
static const std::string ACCOUNT_ID = "101-001-6844455-001";
static const std::string ACCESS_TOKEN = "3481fa8462af186c57b6d6d03a37a0fd-602c4ee66c827bcb87fabddd1da5c094";

// instruments selected
static const std::string INSTRUMENTS[] = {"EUR_USD"};
// granularities selected
static const std::string GRANULARITIES[] = {"D","H4","H1","M1"};

/*-------------------------------------------------------------------------------------------------*/

#include "DateTime.hpp"
#include "Bar.hpp"
//#include "DataBase.hpp"
//#include "OandaAPI.hpp"

//================================================================================================

#endif
