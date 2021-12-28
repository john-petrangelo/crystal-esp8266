#include <gtest/gtest.h>
#include "../Model.ino"

// Demonstrate some basic assertions.
TEST(ModelTest, BasicAssertions) {
  GradientModel gm(4, RED, GREEN, BLUE, RED);

  EXPECT_EQ(gm.apply(0.00 /*pos*/, 0.0 /*ts*/), 0xFF0000);
  EXPECT_EQ(gm.apply(0.25 /*pos*/, 0.0 /*ts*/), 0x40BF00);
  EXPECT_EQ(gm.apply(0.50 /*pos*/, 0.0 /*ts*/), 0x00807F);
  EXPECT_EQ(gm.apply(0.75 /*pos*/, 0.0 /*ts*/), 0x3F00C0);
  EXPECT_EQ(gm.apply(1.00 /*pos*/, 0.0 /*ts*/), 0xFF0000);
}
