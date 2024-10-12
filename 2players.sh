make -j12 -s all
(./bin/Debug/TrollsVsElves server) && sleep 1 & ./bin/Debug/TrollsVsElves client & ./bin/Debug/TrollsVsElves client