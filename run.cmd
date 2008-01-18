@set DIRT_PATH_OLD=%PATH%
@set PATH=%PATH%;%JAVA_HOME%\bin;%JAVA_HOME%\jre\bin
java -jar dist/Dirt.jar
@set PATH=%DIRT_PATH_OLD%
@set DIRT_PATH_OLD=
