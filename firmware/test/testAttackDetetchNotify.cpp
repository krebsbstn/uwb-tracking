#include <DW3000Handler.h>
#include <UWBObserver.h>
#include <unity.h>



class UWBTester : public UWBObserver
{
public:
    UWBTester(UWBObserver::Address address)
    :m_address(address)
    {};
    
    ~UWBTester(){};

    void update(UWBObserver::Address source, uint8_t* data, uint8_t length)
    {
        TEST_ASSERT_EQUAL(0x00, source);

         uint8_t expected[] = {'T', 'e', 's', 't', ' ', 'f', 'r', 'o', 'm', ' ', 'M', 'a', 's', 't', 'e', 'r', '.'};
        for (int i = 0; i < length; i++) {
            TEST_ASSERT_EQUAL_UINT8(expected[i], data[i]);
        }   
    };

private:
    UWBObserver::Address m_address;
};

DW3000Handler dw3000Handler;

void testAttachDetachNotify() {
    UWBTester tester1((UWBObserver::Address)0xFFFF);
    UWBTester tester2((UWBObserver::Address)0xFFFE);
    dw3000Handler.attach(&tester1);
    dw3000Handler.attach(&tester2);
    TEST_ASSERT_EQUAL(2, dw3000Handler.getNumObservers());

    dw3000Handler.detach(&tester1);
    TEST_ASSERT_EQUAL(1, dw3000Handler.getNumObservers());

    dw3000Handler.notify(&tester1);
}

int setup() {
    UNITY_BEGIN();

    RUN_TEST(testAttachDetachNotify);

    UNITY_END();

    return 0;
}

void loop() {
}