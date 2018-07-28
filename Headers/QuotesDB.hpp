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

// SQLite header
#include <sqlite3.h>

#include "Config.hpp"

/*-------------------------------------------------------------------------------------------------*/

// instruments selected
static const std::string INSTRUMENTS[] = {"EUR_USD"};
// granularities selected
static const std::string GRANULARITIES[] = {"D","H4","H1","M1"};

/*-------------------------------------------------------------------------------------------------*/

#include "DateTime.hpp"
#include "Bar.hpp"
#include "DataBase.hpp"
#include "OandaAPI.hpp"

//================================================================================================

#endif
