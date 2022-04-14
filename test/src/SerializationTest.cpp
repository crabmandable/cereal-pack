#include <gtest/gtest.h>
#include <cereal_pack.hpp>
#include <cereal_pack_test/test/SimpleTest.hpp>
#include <cereal_pack_test/test/nesting/Nesting.hpp>
#include <OneBool.hpp>

using namespace cereal_pack_test::test;
using namespace cereal_pack_test::test::nesting;

class SerializationTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
    void PopulateSimpleTest(SimpleTest& st) {
        uint8_t buff[100];
        memset(buff, 0xcc, sizeof(buff));

        OneBool obTrue;
        obTrue.boolean().set(true);
        OneBool obFalse;
        obFalse.boolean().set(false);

        st.string().set("Yo");
        st.boolean().set(true);
        st.uint8().set(22);
        st.int8().set(-22);

        st.uint16().set(288);
        st.int16().set(-288);

        st.uint32().set(22231);
        st.int32().set(-22231);

        st.uint64().set(22231214);
        st.int64().set(-22231214);

        st.dynamic_length_buffer().set(buff, SimpleTest::constants::dynamic_length_buffer_max_length);
        st.const_length_buffer().set(buff);

        st.reference().boolean().set(true);

        st.set_of_bools().push_back(true);
        st.set_of_bools().push_back(false);
        st.set_of_bools().push_back(true);

        st.set_of_references().push_back(obTrue);
        st.set_of_references().push_back(obFalse);
        st.set_of_references().push_back(obTrue);

        st.set_of_buffers().resize(3);
        st.set_of_buffers()[0].set(buff);
        st.set_of_buffers()[1].set(buff);
        st.set_of_buffers()[2].set(buff);
    }
};

TEST_F(SerializationTest, SerializeOneBool) {
    OneBool inOB;
    inOB.boolean().set(true);

    std::vector<uint8_t> buffer;
    buffer.resize(inOB.max_serial_length());

    auto len = inOB.serialize(buffer.data());
    EXPECT_EQ(len, 1);

    OneBool outOB;
    len = outOB.deserialize(buffer.data());
    EXPECT_EQ(len, 1);

    EXPECT_EQ(outOB, inOB);
}

TEST_F(SerializationTest, SerializeSimple) {
    SimpleTest inST;;
    PopulateSimpleTest(inST);

    std::vector<uint8_t> buffer;
    buffer.resize(inST.max_serial_length());

    auto len = inST.serialize(buffer.data());
    EXPECT_EQ(len, inST.serial_length());
    EXPECT_EQ(len, 177);

    SimpleTest outST;
    len = outST.deserialize(buffer.data());
    EXPECT_EQ(len, 177);

    EXPECT_EQ(inST, outST);
}

TEST_F(SerializationTest, SerializeNested) {
    Nesting inNest;
    inNest.bool_ref().boolean().set(true);
    PopulateSimpleTest(inNest.simple_ref());
    inNest.set_of_bool().resize(2);
    inNest.set_of_bool()[0].boolean().set(false);
    inNest.set_of_bool()[1].boolean().set(true);
    inNest.set_of_simple().resize(2);
    PopulateSimpleTest(inNest.set_of_simple()[0]);
    PopulateSimpleTest(inNest.set_of_simple()[1]);

    std::vector<uint8_t> buffer;
    buffer.resize(inNest.max_serial_length());

    auto len = inNest.serialize(buffer.data());
    EXPECT_EQ(len, inNest.serial_length());
    // 3 simples, 2 bools, and 2 length lengths
    EXPECT_EQ(len, 177 * 3 + 3 + 4 * 2 );

    Nesting outNest;
    len = outNest.deserialize(buffer.data());
    EXPECT_EQ(len, 177 * 3 + 3 + 4 * 2);

    EXPECT_EQ(inNest, outNest);
}
