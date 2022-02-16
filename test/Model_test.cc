#include <gtest/gtest.h>
#include "../Model.ino"

// Verify the GradientModel class
TEST(ModelTest, VerifySolidModel) {
  SolidModel sm(RED);

  EXPECT_EQ(sm.apply(0.00 /*pos*/, 0.0 /*ts*/), RED);
  EXPECT_EQ(sm.apply(0.75 /*pos*/, 0.0 /*ts*/), RED);
  EXPECT_EQ(sm.apply(1.00 /*pos*/, 0.0 /*ts*/), RED);
}

// Verify the GradientModel class
TEST(ModelTest, VerifyGradientModel) {
  GradientModel gm(4, RED, GREEN, BLUE, RED);

  EXPECT_EQ(gm.apply(0.00 /*pos*/, 0.0 /*ts*/), 0xFF0000);
  EXPECT_EQ(gm.apply(0.25 /*pos*/, 0.0 /*ts*/), 0x40BF00);
  EXPECT_EQ(gm.apply(0.50 /*pos*/, 0.0 /*ts*/), 0x00807F);
  EXPECT_EQ(gm.apply(0.75 /*pos*/, 0.0 /*ts*/), 0x3F00C0);
  EXPECT_EQ(gm.apply(1.00 /*pos*/, 0.0 /*ts*/), 0xFF0000);
}

// Verify the Rotate class
TEST(ModelTest, VerifyRotate) {
  GradientModel gm(4, RED, GREEN, BLUE, RED);
  Rotate rm_up(&gm, 1.0);
  EXPECT_EQ(rm_up.apply(0.0 /*pos*/, 0.00 /*ts*/), gm.apply(0.00, 0.0));
  EXPECT_EQ(rm_up.apply(0.0 /*pos*/, 0.25 /*ts*/), gm.apply(0.25, 0.0));
  EXPECT_EQ(rm_up.apply(0.0 /*pos*/, 0.50 /*ts*/), gm.apply(0.50, 0.0));
  EXPECT_EQ(rm_up.apply(0.0 /*pos*/, 0.75 /*ts*/), gm.apply(0.75, 0.0));
  EXPECT_EQ(rm_up.apply(0.0 /*pos*/, 1.00 /*ts*/), gm.apply(1.00, 0.0));

  Rotate rm_down(&gm, 1.0);
  EXPECT_EQ(rm_down.apply(0.0 /*pos*/, 0.00 /*ts*/), gm.apply(0.00, 0.0));
  EXPECT_EQ(rm_down.apply(0.0 /*pos*/, 0.25 /*ts*/), gm.apply(0.75, 0.0));
  EXPECT_EQ(rm_down.apply(0.0 /*pos*/, 0.50 /*ts*/), gm.apply(0.50, 0.0));
  EXPECT_EQ(rm_down.apply(0.0 /*pos*/, 0.75 /*ts*/), gm.apply(0.25, 0.0));
  EXPECT_EQ(rm_down.apply(0.0 /*pos*/, 1.00 /*ts*/), gm.apply(0.00, 0.0));
}
