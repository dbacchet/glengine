#include <iostream>
#include <thread>

#include "gtest/gtest.h"

#include "grid.h"
#include "stb/stb_image_write.h"

namespace {
}


TEST(Grid2D, Creation) {
    Grid2D<double> grid;
    auto s = grid.size();
    ASSERT_EQ(s.first, 1);
    ASSERT_EQ(s.second, 1);
    grid.init(0.0, 0.0, 10.0, 11.0, 1.0);
    s = grid.size();
    ASSERT_EQ(s.first, 10);
    ASSERT_EQ(s.second, 11);
    grid.init(0.0, 0.0, 10.0, 11.0, 0.1);
    s = grid.size();
    ASSERT_EQ(s.first, 100);
    ASSERT_EQ(s.second, 110);
    // ASSERT_TRUE(true);
    // ASSERT_FALSE(false);
    // ASSERT_EQ(idtable.size(),0);
}

TEST(Grid2D, Access) {
    // by index
    Grid2D<double> grid;
    grid.init(0,0,10,5,1.0);
    auto c = grid.at_pos_safe(1,2);
    ASSERT_TRUE(c);
    c = grid.at_index_safe(13,2);
    ASSERT_FALSE(c);
    c = grid.at_index_safe(1,1);
    ASSERT_FLOAT_EQ(*c,0.0);
    grid.at_index(1,1) = 1.23; // set value using the returned reference
    c = grid.at_index_safe(1,1);
    ASSERT_FLOAT_EQ(*c,1.23);
    // by position
    grid.init(3,-1,9,5,1.0);
    // set data by index
    for (int i=0; i<10; i++) {
        for (int j=0; j<5; j++) {
            grid.at_index(i,j) = j*10+i;
        }
    }
    c = grid.at_pos_safe(3.0,-1.0);
    ASSERT_TRUE(c);
    c = grid.at_pos_safe(2,2); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(13,2); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(4,5); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(4,-1.1); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(1,1);
    ASSERT_FLOAT_EQ(*c,0.0);
    grid.at_pos(3.5,1) = 1.23; // set value using the returned reference
    c = grid.at_pos_safe(3.5,1);
    ASSERT_TRUE(c);
    ASSERT_FLOAT_EQ(*c,1.23);
    // grid with a smaller cell size
    grid.init(3,-1,9,5,0.15);
    for (int i=0; i<10; i++) {
        for (int j=0; j<5; j++) {
            grid.at_index(i,j) = j*10+i;
        }
    }
    c = grid.at_pos_safe(3.0,-1.0);
    ASSERT_TRUE(c);
    c = grid.at_pos_safe(2,2); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(13,2); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(4,5); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(4,-1.1); // out of bounds
    ASSERT_FALSE(c);
    c = grid.at_pos_safe(1,1);
    ASSERT_FLOAT_EQ(*c,0.0);
    grid.at_pos(3.5,1) = 1.23; // set value using the returned reference
    c = grid.at_pos_safe(3.5,1);
    ASSERT_TRUE(c);
    ASSERT_FLOAT_EQ(*c,1.23);

    // auto gridsize = grid.size();
    // stbi_write_png("grid_image.png", gridsize.first, gridsize.second, 1, (void*)grid.data, int stride_in_bytes);
}

TEST(Grid2D, Set) {
    Grid2D<double> grid;
    grid.init(3,-1,9,5,0.5);
    ASSERT_TRUE(grid.set_at_pos_safe(3,-1,0.8));
    ASSERT_TRUE(grid.set_at_pos_safe(3.5,0.0,0.7));
    ASSERT_TRUE(grid.set_at_pos_safe(3.8,0.7,0.6));
    ASSERT_TRUE(grid.set_at_pos_safe(3.8,0.8,0.5)); // this will overwrite the previous
    ASSERT_TRUE(grid.set_at_pos_safe(4.1,1.1,0.4));
    ASSERT_FALSE(grid.set_at_pos_safe(12.1,1.1,0.4));
    ASSERT_FLOAT_EQ(grid.at_pos(3,-1),0.8);
    ASSERT_FLOAT_EQ(grid.at_pos(3.5,0.0),0.7);
    ASSERT_FLOAT_EQ(grid.at_pos(3.8,0.7),0.5);
    ASSERT_FLOAT_EQ(grid.at_pos(3.8,0.8),0.5);
    ASSERT_FLOAT_EQ(grid.at_pos(4.1,1.1),0.4);
}

