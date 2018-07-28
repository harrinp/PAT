COMPILER = g++
CCFLAGS1 = -lPocoNet -lPocoNetSSL -lPocoFoundation -lPocoJSON -lmysqlcppconn -lssl -lcrypto -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -I/usr/include/cppconn -std=c++11 -lcurl -I./Headers -lsqlite3
CCFLAGS2 = -lPocoNet -lPocoNetSSL -lPocoFoundation -lPocoJSON -lmysqlcppconn -lssl -lcrypto -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -std=c++11 -lcurl -lsqlite3

decide: Decision.cpp TestExec.cpp
	${COMPILER} ${CCFLAGS2} NewDecision/Decider.cpp NewDecision/Tester.cpp NewDecision/Runner.cpp TestExec.cpp Executor.cpp Trade.cpp -o runD.out
analyze: Analysis.cpp
	${COMPILER} ${CCFLAGS2} Analysis.cpp -o runA.out
execute: Executor.cpp
	${COMPILER} ${CCFLAGS2} Executor.cpp Trade.cpp -o runX.out
example: example.cpp
	${COMPILER} ${CCFLAGS2} example.cpp -o runE.out

# Use these ones for if your Ubuntu install is unhappy
decideU: Decision.cpp TestExec.cpp
	${COMPILER} Decision.cpp TestExec.cpp Executor.cpp Trade.cpp -o runD.out ${CCFLAGS1} -L/usr/lib -lmysqlcppconn
analyzeU: Analysis.cpp
	${COMPILER} Analysis.cpp -o runA.out ${CCFLAGS1} -L/usr/lib -lmysqlcppconn -lPocoNet -lPocoNetSSL
executeU: Executor.cpp
	${COMPILER} Executor.cpp Trade.cpp -o runX.out ${CCFLAGS1}
exampleU: example.cpp
	${COMPILER} example.cpp -o runE.out ${CCFLAGS1}
