#include "Tools/Buffer.h"
#include <gtest/gtest.h>

class BufferTest : public ::testing::Test{
};



TEST_F(BufferTest, AppendAndRetrieveTest){
    Buffer buf;
    ASSERT_EQ(buf.ReadableBytes(), 0) << "error: ReadableBytes is " << buf.ReadableBytes();
    ASSERT_EQ(buf.WritableBytes(), Buffer::kInitialSize) << "error: WritableBytes is " << buf.WritableBytes();
    ASSERT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend) << "error: PrepenableBytes is " << buf.PrepenableBytes();

    std::string str1(200, 'x');
    buf.Append(str1);
    EXPECT_EQ(buf.ReadableBytes(), str1.size());
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - str1.size());
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend);

    std::string str2 = buf.RetrieveAsString(50);
    EXPECT_EQ(str2.size(), 50);
    EXPECT_EQ(str2, std::string(50, 'x'));
    EXPECT_EQ(buf.ReadableBytes(), str1.size() - str2.size());
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - str1.size());
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend + str2.size());

    std::string str3(150, 'y');
    buf.Append(str3.c_str(), 50);
    EXPECT_EQ(buf.ReadableBytes(), str1.size() - str2.size() + 50);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - str1.size() - 50);
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend + str2.size());

    const void* str3_ptr = str3.c_str() + 50;
    buf.Append(str3_ptr, 100);
    EXPECT_EQ(buf.ReadableBytes(), str1.size() - str2.size() + str3.size());
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - str1.size() - str3.size());
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend + str2.size());

    buf.Retrieve(100);
    EXPECT_EQ(buf.ReadableBytes(), str1.size() - str2.size() + str3.size() - 100);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - str1.size() - str3.size());
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend + str2.size() + 100);

    buf.RetrieveUntil(buf.Peek() + 20);
    EXPECT_EQ(buf.ReadableBytes(), str1.size() - str2.size() + str3.size() - 120);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - str1.size() - str3.size());
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend + str2.size() + 120);

    buf.RetrieveUntil(buf.Peek() + buf.ReadableBytes());
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize);
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend);
}


TEST_F(BufferTest, PeekTest){
    Buffer buf;
    std::string str1(200, 'x');
    buf.Append(str1);
    std::string str2(buf.Peek());
    EXPECT_EQ(str1, str2);
}

TEST_F(BufferTest, GrowTest){
    Buffer buf;
    buf.Append(std::string(800, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 800);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - 800);
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend);

    buf.Append(std::string(800, 'z'));
    size_t new_size = buf.WritableBytes() + buf.ReadableBytes() + buf.PrepenableBytes();
    EXPECT_EQ(buf.ReadableBytes(), 1600);
    EXPECT_EQ(buf.WritableBytes(), 0);
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend);
    EXPECT_EQ(new_size, 1600 + Buffer::kCheapPrepend);

}

TEST_F(BufferTest, InsideGrowTest){
    Buffer buf;
    buf.Append(std::string(1000, 'z'));
    EXPECT_EQ(buf.ReadableBytes(), 1000);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - 1000);
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend);

    buf.Retrieve(500);
    EXPECT_EQ(buf.ReadableBytes(), 500);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - 1000);
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend + 500);

    buf.Append(std::string(300,'x'));
    EXPECT_EQ(buf.ReadableBytes(), 800);
    EXPECT_EQ(buf.WritableBytes(), Buffer::kInitialSize - 800);
    EXPECT_EQ(buf.PrepenableBytes(), Buffer::kCheapPrepend);
}