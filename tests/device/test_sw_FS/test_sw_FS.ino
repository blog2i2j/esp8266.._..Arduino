#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <BSTest.h>

BS_ENV_DECLARE();

void setup()
{
    Serial.begin(115200);
    BS_RUN(Serial);
}

bool pretest()
{
    return true;
}


TEST_CASE("read-write test","[fs]")
{
    REQUIRE(LittleFS.begin());

    String text = "write test";
    {
        File out = LittleFS.open("/tmp.txt", "w");
        REQUIRE(out);
        out.print(text);
    }

    {
        File in = LittleFS.open("/tmp.txt", "r");
        REQUIRE(in);
        CHECK(in.size() == text.length());
        in.setTimeout(0);
        String result = in.readString();
        CHECK(result == text);
    }
}

TEST_CASE("A bunch of files show up in openDir, and can be removed", "[fs]")
{
    REQUIRE(LittleFS.begin());
    const int n = 10;
    int found[n] = {0};

    {
        Dir root = LittleFS.openDir("");
        while (root.next()) {
            CHECK(LittleFS.remove(root.fileName()));
        }
    }

    for (int i = 0; i < n; ++i) {
        String name = "/seq_";
        name += i;
        name += ".txt";

        File out = LittleFS.open(name, "w");
        REQUIRE(out);

        out.println(i);
    }

    {
        Dir root = LittleFS.openDir("/");
        while (root.next()) {
            String fileName = root.fileName();
            CHECK(fileName.indexOf("/seq_") == 0);
            int i = fileName.substring(5).toInt();
            CHECK(i >= 0 && i < n);
            found[i]++;
        }

        for (auto f : found) {
            CHECK(f == 1);
        }
    }

    {
        Dir root = LittleFS.openDir("/");
        while (root.next()) {
            String fileName = root.fileName();
            CHECK(LittleFS.remove(fileName));
        }
    }
}

TEST_CASE("files can be renamed", "[fs]")
{
    REQUIRE(LittleFS.begin());
    {
        File tmp = LittleFS.open("/tmp1.txt", "w");
        tmp.println("rename test");
    }

    {
        CHECK(LittleFS.rename("/tmp1.txt", "/tmp2.txt"));

        File tmp2 = LittleFS.open("/tmp2.txt", "r");
        CHECK(tmp2);
    }
}

TEST_CASE("FS::info works","[fs]")
{
    REQUIRE(LittleFS.begin());
    FSInfo info;
    CHECK(LittleFS.info(info));

    Serial.printf("Total: %u\nUsed: %u\nBlock: %u\nPage: %u\nMax open files: %u\nMax path len: %u\n",
                  info.totalBytes,
                  info.usedBytes,
                  info.blockSize,
                  info.pageSize,
                  info.maxOpenFiles,
                  info.maxPathLength
                 );
}

TEST_CASE("FS is empty after format","[fs]")
{
    REQUIRE(LittleFS.begin());
    REQUIRE(LittleFS.format());

    Dir root = LittleFS.openDir("/");
    int count = 0;
    while (root.next()) {
        ++count;
    }
    CHECK(count == 0);
}

TEST_CASE("Can reopen empty file","[fs]")
{
    REQUIRE(LittleFS.begin());
    {
        File tmp = LittleFS.open("/tmp.txt", "w");
    }
    {
        File tmp = LittleFS.open("/tmp.txt", "w");
        CHECK(tmp);
    }
}

void loop()
{
}
