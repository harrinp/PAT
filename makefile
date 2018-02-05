COMPILER = g++
CCFLAGS1 = -lPocoNet -lPocoNetSSL -lPocoFoundation -lPocoJSON -lmysqlcppconn -lssl -lcrypto -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto
CCFLAGS2 = -lPocoNet -lPocoNetSSL -lPocoFoundation -lPocoJSON -lmysqlcppconn -lssl -lcrypto -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -std=c++11 -lcurl

decide: Decision.cpp TestExec.cpp
	${COMPILER} ${CCFLAGS2} Decision.cpp TestExec.cpp Trade.cpp -o runD.out
analyze: Analysis.cpp
	${COMPILER} ${CCFLAGS2} Analysis.cpp -o runA.out
execute: Executor.cpp
	${COMPILER} ${CCFLAGS2} Executor.cpp Trade.cpp -o runX.out
example: example.cpp
	${COMPILER} ${CCFLAGS2} example.cpp -o runE.out
