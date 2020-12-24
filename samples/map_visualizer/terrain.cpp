#include "terrain.h"

#include "math/math_utils.h"
#include "math/vmath.h"
#include "wgs84_converter.hpp"

#include "curl/curl.h"
#include "json.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

using nlohmann::json;
namespace fs = std::filesystem;

namespace {

size_t append_to_string(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
template <typename T>
void calc_barycentric_coords(math::Vector2<T> p, math::Vector2<T> a, math::Vector2<T> b, math::Vector2<T> c, T &u, T &v,
                             T &w) {
    math::Vector2<T> v0 = b - a;
    math::Vector2<T> v1 = c - a;
    math::Vector2<T> v2 = p - a;
    T d00 = v0.dot(v0);
    T d01 = v0.dot(v1);
    T d11 = v1.dot(v1);
    T d20 = v2.dot(v0);
    T d21 = v2.dot(v1);
    T denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = T(1.0) - v - w;
}

} // namespace

class HttpCache {
  public:
    HttpCache() { curl_global_init(CURL_GLOBAL_ALL); }
    virtual ~HttpCache() {
        // always cleanup
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    bool init(const std::string &cache_path) {
        curl = curl_easy_init();
        cache_dir = cache_path;
        fs::create_directories(cache_dir);
        return curl;
    }

    bool get(const std::string &url, std::string &response) {
        if (!curl) {
            return false;
        }
        // check if the url is in the cache
        size_t url_hash = std::hash<std::string>{}(url);
        std::stringstream cache_entry;
        cache_entry << cache_dir << "/" << url_hash << ".response";
        if (fs::exists(cache_entry.str())) {
            std::cout << "fetching request" /* << url */ << " from cache file " << cache_entry.str() << std::endl;
            std::ifstream f(cache_entry.str().c_str()); // open the input file
            std::stringstream fss;
            fss << f.rdbuf();
            response = fss.str();
            f.close();
        } else {
            // make a regular curl call and store the result in the cache
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_to_string);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            CURLcode res = curl_easy_perform(curl);
            std::ofstream of(cache_entry.str().c_str());
            of << response;
            of.close();
            std::cout << "created cache entry: " << cache_entry.str() << std::endl;
        }

        return true;
    }

    CURL *curl = nullptr;
    std::string cache_dir = "terrain_cache";
};

bool Terrain::fetch_elevation_data(const math::Vector3d &origin) {
    char *elev_api_key = getenv("GOOGLE_MAPS_API_KEY");
    if (!elev_api_key) {
        std::cerr << "Google Maps Api Key environment variable missing. Terrain elevation mesh has been disabled\n";
        return false;
    }
    std::cout << "fetch elev data" << std::endl;
    WGS84Converter conv;
    conv.set_origin(origin);

    // build the matrix of points we want to query
    int32_t nx = 2 * radius + 1;
    int32_t ny = 2 * radius + 1;
    grid.clear();
    grid.resize(nx * ny);
    std::vector<math::Vector3d> grid_lle(grid.size());
    for (int32_t j = 0; j < ny; j++) {
        for (int32_t i = 0; i < nx; i++) {
            math::Vector3d pt{(i - radius) * tile_len, (j - radius) * tile_len, 0.0};
            grid[j * nx + i] = pt;
            grid_lle[j * nx + i] = conv.cart_to_wgs84(pt);
        }
    }

    // query the map elevatrion API
    HttpCache http_cache;
    if (!http_cache.init(".terrain_cache")) {
        std::cerr << "error initializing terrain cache" << std::endl;
        return false;
    }

    int idx = 0;
    while (idx < nx * ny) {
        uint32_t batch_len = 150; // to make sure the request length is below the 8k limit of google elev api
        std::stringstream ss;
        char tmp[1024];
        int num_queries = 0;
        for (num_queries = 0; num_queries < batch_len; num_queries++) {
            if (idx + num_queries >= nx * ny) {
                break;
            }
            const auto &lle = grid_lle[idx + num_queries];
            sprintf(tmp, "|%.18f,%.18f", lle.x, lle.y);
            ss << tmp;
        }
        std::stringstream url;
        url << "https://maps.googleapis.com/maps/api/elevation/json?locations=" << ss.str().substr(1)
            << "&key=" << std::string(elev_api_key);
        std::string response;
        if (!http_cache.get(url.str(), response)) {
            std::cerr << "error executing request: " << url.str() << std::endl;
        }

        json data = json::parse(response);
        if (data["status"] == "OK") {
            const auto &results = data["results"];
            if (results.size() != num_queries) {
                std::cerr << "wrong number of results in the query: expected " << num_queries << " but got "
                          << results.size() << std::endl;
                return false;
            }
            for (int i = 0; i < results.size(); i++) {
                const auto &v = results[i];
                math::Vector3d lle = {v["location"]["lat"], v["location"]["lng"], v["elevation"]};
                auto pp = math::Vector3f(conv.wgs84_to_cart(lle));
                constexpr double Z_OFFSET_TO_MATCH_PCDS = 33.028;
                grid[idx + i].z = pp.z - Z_OFFSET_TO_MATCH_PCDS;
            }
        }
        idx += batch_len;
    }

    // create a gfx grid
    std::vector<glengine::Vertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(grid.size());
    indices.reserve(grid.size());
    for (const auto &p : grid) {
        vertices.push_back({math::Vector3f(p)});
    }
    for (uint32_t i = 0; i < nx - 1; i++) { // lines for the last row/col are missing
        for (uint32_t j = 0; j < ny - 1; j++) {
            indices.push_back(j * nx + i);
            indices.push_back(j * nx + i + 1);
            indices.push_back(j * nx + i);
            indices.push_back((j + 1) * nx + i);
        }
    }

    auto points_mesh = _eng.resource_manager().create_mesh("terrain");
    points_mesh->init(vertices, indices, GL_LINES);
    glengine::Renderable points_renderable = {points_mesh, terrain_material};
    auto points_ro = _eng.create_renderobject(points_renderable, root);

    return true;
}

math::Vector3f Terrain::interpolate(double x_, double y_) {
    // limit to the borders
    const double x = math::utils::clamp(x_, -radius * tile_len, radius * tile_len);
    const double y = math::utils::clamp(y_, -radius * tile_len, radius * tile_len);
    // find the indices
    int32_t i = math::utils::clamp((int32_t)std::floor(x / tile_len) + radius, 0, 2 * radius - 1);
    int32_t j = math::utils::clamp((int32_t)std::floor(y / tile_len) + radius, 0, 2 * radius - 1);
    const double dx = x - (i-radius) * tile_len;
    const double dy = y - (j-radius) * tile_len;
    // calc barycentric coordinates
    int32_t nx = 2 * radius + 1;
    double a,b,c;
        auto p0 = grid[j*nx+i];
        auto p1 = grid[j*nx+i+1];
        auto p2 = grid[(j+1)*nx+i+1];
    if (dy>dx) {
        p0 = grid[j*nx+i];
        p1 = grid[(j+1)*nx+i+1];
        p2 = grid[(j+1)*nx+i];
    }
    calc_barycentric_coords(math::Vector2d(x,y), math::Vector2d(p0.x,p0.y), math::Vector2d(p1.x,p1.y), math::Vector2d(p2.x,p2.y), a, b, c);
    auto p = a*p0 + b*p1 + c*p2;
    // printf("%.2f,%.2f %.2f %.2f | (%.2f,%.2f,%.2f) (%.2f,%.2f,%.2f) (%.2f,%.2f,%.2f) -> (%.2f,%.2f,%.2f)\n",x,y, dx,dy, p0.x,p0.y,p0.z, p1.x,p1.y,p1.z, p2.x,p2.y,p2.z, p.x,p.y,p.z);
    // printf("%f %f %f\n", a,b,c);
    return p;
}
