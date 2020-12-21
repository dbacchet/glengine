#include "gl_types.h"

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include <vector>

// load points for the given file, and add them to the vector
bool load_pcd(const char *filename, std::vector<glengine::Vertex> &points) {
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

    if (pcl::io::loadPCDFile<pcl::PointXYZ>(filename, *cloud) == -1) //* load the file
    {
        std::cerr << "Error reading file " << filename << std::endl;
        return false;
    }
    // std::cout << "Loaded " << cloud->width * cloud->height << " data points from " << std::string(filename) << std::endl;
    for (const auto &point : *cloud) {
        points.push_back({{point.x, point.y, point.z}, {255, 255, 255, 255}});
    }

    return true;
}

