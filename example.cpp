//=================================================================================================
//                    Copyright (C) 2017 Olivier Mallet - All Rights Reserved
//=================================================================================================

#include "Headers/QuotesDB.hpp"
#include "Headers/Config.hpp"

int main()
{
   // creating a connection to Oanda server
   qdb::OandaAPI conn("practice");



   // downloading data from the 1st of Janurary 2007 and recording them in QuotesDB database
   conn.initAllTabs(DATABASE_NAME,"2007-01-01 00:00:00");
   // it will then download data for every pair (INSTRUMENT,GRANULARITY) defined in QuotesDB.hpp

   // you can the update later the database by simply doing:
   //conn.updateAllTabs("QuotesDB");

   //conn.updateTab("QuotesDB", "EUR_USD", "M1");

   // connecting to QuotesDB database for reading data

   //std::cout << conn.getInstruments() << '\n';

   //qdb::DataBase db(DATABASE);

   // qdb::Bar b = db.get_last_row("EUR_USD_D");

   // printf("%d\n", b.date );

   // reading the last 10 recorded Bars from table EUR_USD_D
   //std::vector<qdb::Bar> data = db.read_table("EUR_USD_M1",10);

   // outputting the data
   // for (int i = 0; i < data.size(); ++i) {
   //    data[i].print();
   // }
}
