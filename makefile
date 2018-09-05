COMPILER = g++
CCFLAGS1 = -lPocoNet -lPocoNetSSL -lPocoFoundation -lPocoJSON -lmysqlcppconn -lssl -lcrypto -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -I/usr/include/cppconn -std=c++11 -lcurl -I./Headers -lsqlite3
CCFLAGS2 = -lPocoNet -lPocoNetSSL -lPocoFoundation -lPocoJSON -lmysqlcppconn -lssl -lcrypto -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -std=c++11 -lcurl -lsqlite3
CCFLAGS3 = -std=c++11 -lcurl -lsqlite3

decide:
	${COMPILER} RunMain.cpp Decider.cpp Runner.cpp FullBar.cpp Executor.cpp Trade.cpp -o runD.out ${CCFLAGS3}
analyze: Analysis.cpp
	${COMPILER} Analysis.cpp FullBar.cpp Trade.cpp -o runA.out ${CCFLAGS1} -L/usr/lib -lmysqlcppconn -lPocoNet -lPocoNetSSL
test:
	${COMPILER} TestMain.cpp Decider.cpp Tester.cpp FullBar.cpp TestExec.cpp Trade.cpp -o runT.out ${CCFLAGS2}

executeTest: Executor.cpp
	${COMPILER} Executor.cpp Trade.cpp -o runX.out ${CCFLAGS1}
