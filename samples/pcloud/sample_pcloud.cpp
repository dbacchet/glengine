#include "math/vmath.h"

#include "gl_engine.h"
#include "gl_mesh.h"
#include "gl_prefabs.h"
#include "gl_material_diffuse.h"
#include "gl_material_diffuse_textured.h"
#include "gl_material_flat.h"
#include "gl_material_flat_textured.h"
#include "gl_material_vertexcolor.h"
#include "gl_renderable.h"
#include "gl_logger.h"

#include "RHMQ.h"

#include "header_generated.h"
#include "pointcloud_measurement_generated.h"
#include "virtual_sensor_measurement_generated.h"
#include "proxy_to_control_request_generated.h"
#include "control_to_proxy_reply_generated.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>

// data for the point cloud and the object detections
std::vector<glengine::Vertex> pcloud_points;
struct Detection {
    bool visible;
    math::Vector3f center;
    math::Vector3f size;
    glengine::Color color;
};
std::unordered_map<uint32_t, Detection> vsensor_detections;

// threads
std::vector<std::thread> threads;
std::recursive_mutex global_mutex;
bool running = true;

bool handle_pointcloud_message(unsigned char *buffer, int buffer_size) {
    // got message, decode
    auto pc_msg = RHMsg::GetPointCloudMeasurement(buffer);
    // read and validate the header (if it exists, otherwise print warning message)
    if (pc_msg->header()) {
        const RHMsg::HeaderData *header = pc_msg->header();
        // verify the message type is correct
        assert(header->source() == RHMsg::Source::Source_POINTCLOUD_SENSOR);
    } else {
        std::cout << "Warning: received buffer with no header." << std::endl;
    }

    // read pointcloud data
    {
        std::lock_guard<std::recursive_mutex> guard(global_mutex);
        flatbuffers::uoffset_t num_points = pc_msg->points()->size();
        pcloud_points.resize(num_points);

        for (int i = 0; i < num_points; i++) {
            // get a point from the array like so:
            auto point = pc_msg->points()->Get(i);
            // and access its fields like so:
            // the position the beam struck in the sensor's coordinate frame (meters)
            float x = point->x();
            float y = point->y();
            float z = point->z();
            // the intensity of the return (normalized [0, 1])
            float intensity = point->intensity();
            pcloud_points[i] = {
                {x, y, z},
                {uint8_t(255 - intensity * 255), uint8_t(255 - intensity * 255), uint8_t(intensity * 255), 255}};
        }
    }
    return true;
}

bool handle_virtual_sensor_message(unsigned char *buffer, int buffer_size) {
    // got message, decode
    auto *virtual_sensor_msg = RHMsg::GetVirtualSensorMeasurement(buffer);
    // read and validate the header (if it exists, otherwise print warning message)
    if (virtual_sensor_msg->header()) {
        const RHMsg::HeaderData *header = virtual_sensor_msg->header();

        // verify the message type is correct
        assert(header->source() == RHMsg::Source::Source_VIRTUAL_SENSOR);
    } else {
        std::cout << "Warning: received buffer with no header." << std::endl;
    }

    // the virtual sensor message is in three parts: lane markings, signs, and obstacles
    {
        std::lock_guard<std::recursive_mutex> guard(global_mutex);

        // hide all previous detections
        for (auto &it : vsensor_detections) {
            it.second.visible = false;
        }

        // signs
        flatbuffers::uoffset_t num_signs = virtual_sensor_msg->signs()->size();
        for (flatbuffers::uoffset_t i = 0; i < num_signs; i++) {
            // get a sign from the array like so:
            const RHMsg::Sign *sign = virtual_sensor_msg->signs()->Get(i);
            // and access its fields like so:
            // an id for the sign; these are persistent between simulations in the same environment,
            // however multiple entries may have the same id, meaning the signs had multiple type
            // that were co-located, for example a pedestrian crossing with an arrow would generate
            // two entries in this signs vector with the same "id" field and different "type" fields
            uint32_t id = sign->id();
            // the center of the sign's bounding box, in the sensor's coordinate frame (meters)
            const RHMsg::Vec3 *center = sign->center();
            // the height of the bounding box (meters)
            float height = sign->height();
            // the width of the bounding box (meters)
            float width = sign->width();
            // the sign type, as a string
            const flatbuffers::String *type = sign->type();
            // convert to native string types like so:
            const char *type_as_c_str = type->c_str();
            std::string type_as_std_str = type->str();
            // the sensor's confidence in this measurement (in [0, 3])
            int32_t confidence = sign->confidence();
            // record detection
            auto &s = vsensor_detections[id];
            s.visible = true;
            s.center = {center->x(), center->y(), center->z()};
            s.size = {1.0f, width, height};
            s.color = {255, 0, 0, 255};
        }

        // obstacles, a vector of detected obstacles
        flatbuffers::uoffset_t num_obstacles = virtual_sensor_msg->obstacles()->size();
        for (int i = 0; i < num_obstacles; i++) {
            // get an obstacle from the array like so:
            const RHMsg::Obstacle *obstacle = virtual_sensor_msg->obstacles()->Get(i);
            // and access its fields like so:
            // an id for the obstacle; these are guaranteed to be persistent over the course of a
            // simulation however the are NOT guaranteed to be persistent between simulations
            // of the same scenario; for a use case requiring persistence across different
            // runs of the same scenario, use the UUIDs, which match the UUID in the scenario
            uint32_t id = obstacle->id();
            // the UUID, which matches the agent's UUID in the scenario
            const RHMsg::UUID uuid = obstacle->uuid();
            // the center of the obstacle's bounding box, in the sensor's coordinate frame (meters)
            const RHMsg::Vec3 center = obstacle->center();
            // the height of the bounding box (meters)
            float height = obstacle->height();
            // the width of the bounding box (meters)
            float width = obstacle->width();
            // the obstacle type, as an enum
            // (RHMsg::ObstacleType_PEDESTRIAN, RHMsg::ObstacleType_VEHICLE, or
            // RHMsg::ObstacleType_UNKNOWN)
            const RHMsg::ObstacleType type = obstacle->type();
            // the sensor's confidence in this measurement (in [0, 3])
            int32_t confidence = obstacle->confidence();
            // record detection
            auto &s = vsensor_detections[id];
            s.visible = true;
            s.center = {center.x(), center.y(), center.z()};
            s.size = {1.0f, width, height};
            s.color = {255, 0, 255, 255};
        }
    }
    return true;
}

bool dummy_handler(unsigned char *buffer, int buffer_size) {
    return true;
}

// function used in each thread receiving data from a sensor
void thread_fun(const std::string &endpoint, int zmq_type, std::function<bool(uint8_t *, int)> handler) {
    RHMQ_Socket *socket = RHMQ::CreateSocket();
    socket->Init(zmq_type, endpoint.c_str());
    unsigned char *buffer = 0;
    while (running) {
        int buffer_length = socket->Receive(buffer, 100 /* optional timeout (ms) */);
        if (buffer_length < 0) {
            // this is an error condition
            log_error("Error receiving on socket %s", endpoint.c_str());
        } else if (buffer_length == 0) {
            // this means we got through timeout and there was no message waiting for us
        } else {
            // message must be good, so handle it
            assert(buffer_length > 0);
            bool handled = handler(buffer, buffer_length);
            if (!handled) {
                // report the error but do nothing
                log_error("Error handling message on socket %s", endpoint.c_str());
            }
        }
    }
}

// step the sim time when in sync mode
bool advance_time(RHMQ_Socket *sim_control_socket) {
    flatbuffers::FlatBufferBuilder fbb;
    int received_buf_len = 0;
    // pointer for received buffer
    unsigned char *buffer_ptr_sim_control = 0;
    // message received from client
    {
        // Set clock before engine sends states for this frame
        auto request = CreateProxyToControlRequest(fbb, RHMsg::ProxyToControlRequestType_ADVANCE);
        fbb.Finish(request);
    }
    // send message to sim control server
    int send_bytes = sim_control_socket->Send(fbb.GetBufferPointer(), fbb.GetSize());
    if (send_bytes == -1) {
        log_error("error sending sim control request");
        return false;
    }
    // receive message from sim control server
    {
        received_buf_len = sim_control_socket->Receive(buffer_ptr_sim_control, RHMQ_TIMEOUT_INF);
        if (received_buf_len > 0) {
            auto flatbuffer_msg = RHMsg::GetControlToProxyReply(buffer_ptr_sim_control);
            auto reply = flatbuffer_msg->reply();
            if (reply == RHMsg::ControlToProxyReplyType_DONE) {
                double sim_control_time = flatbuffer_msg->time();
                log_debug("reply: time advanced to %f", sim_control_time);
            }
        } else {
            log_error("error receiving reply after advance");
            return false;
        }
    }
    return true;
}


int main(int argc, char *argv[]) {

    glengine::GLEngine eng;
    eng.init({1280, 720, true});

    eng._camera_manipulator.set_azimuth(0.5f).set_elevation(0.8f);

    // endpoints
    /// \todo get them from the config file
    std::string endpoint_lidar = "tcp://192.168.0.105:30008";
    std::string endpoint_vsensor = "tcp://192.168.0.105:30009";
    std::string endpoint_simcontrol_req = "tcp://192.168.0.105:30001";
    // additional publishers that need receivers (every data producer needs to have a subscriber in order to step time)
    std::vector<std::string> dummy_endpoints = {"tcp://192.168.0.105:30005", "tcp://192.168.0.105:30006", "tcp://192.168.0.105:30007"};

    // RHMQ
    bool sync = true;
    // create the receivers
    int zmq_type = sync ? ZMQ_PULL : ZMQ_SUB;
    threads.push_back(std::thread(thread_fun, endpoint_lidar, zmq_type, handle_pointcloud_message));
    threads.push_back(std::thread(thread_fun, endpoint_vsensor, zmq_type, handle_virtual_sensor_message));

    // sync execution
    RHMQ_Socket *socket_simcontrol_req = nullptr;
    if (sync) {
        socket_simcontrol_req = RHMQ::CreateSocket();
        if (!socket_simcontrol_req->Init(ZMQ_REQ, endpoint_simcontrol_req.c_str())) {
            log_error("error creating ZMQ_REQ socket on %s", endpoint_simcontrol_req.c_str());
        }
        // dummy sockets (every data producer needs to have a subscriber in order to step time)
        for (const auto &s : dummy_endpoints) {
            threads.push_back(std::thread(thread_fun, s, zmq_type, dummy_handler));
        }
    }

    // /////// //
    // objects //
    // /////// //
    // grid
    auto *grid = eng.create_object({eng.create_grid_mesh(50.0f, 1.0f),
                                    eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES)});
    // boxes
    // mesh
    auto box_md = glengine::create_box_data();
    glengine::Mesh *box_mesh = eng.create_mesh(box_md.vertices, box_md.indices);
    // material
    auto *box_mtl_diffuse =
        eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
    // renderables
    glengine::Renderable box_renderable_diffuse{box_mesh, box_mtl_diffuse};
    // objects
    auto *root = eng.create_object();
    auto *box = eng.create_object(box_renderable_diffuse, root);

    glengine::Renderable axes{eng.create_axis_mesh(), eng.create_material<glengine::MaterialVertexColor>(
                                                          SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    box->add_renderable(&axes, 1);
    box->set_scale({4.5f, 2.1f, 1.5f});
    box->set_transform(math::create_translation<float>({0.0f, 0.0f, -0.75f}));
    grid->set_transform(math::create_translation<float>({0.0f, 0.0f, -1.5f}));

    // /////////// //
    // sensor data //
    // /////////// //

    // mesh with the point cloud
    glengine::Mesh *pcloud_mesh = eng.create_mesh(box_md.vertices, {}, SG_USAGE_DYNAMIC);
    auto *pcloud_mtl = eng.create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_POINTS, SG_INDEXTYPE_NONE);
    auto *pcloud = eng.create_object({pcloud_mesh, pcloud_mtl});
    // detections from virtual sensors
    std::unordered_map<uint32_t, glengine::Object *> bboxes;

    // ///////// //
    // main loop //
    // ///////// //
    int cnt = 0;
    while (running) {
        // hide all previous detections
        for (auto &it : bboxes) {
            it.second->set_visible(false);
        }
        // update the renderer in a critical section
        {
            std::lock_guard<std::recursive_mutex> guard(global_mutex);
            pcloud_mesh->vertices = pcloud_points;
            pcloud->update();
            // bboxes
            for (auto &it : vsensor_detections) {
                uint32_t id = it.first;
                if (bboxes.count(id) == 0) {
                    // create the bounding box object
                    auto *mtl =
                        eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
                    mtl->color = it.second.color;
                    bboxes[id] = eng.create_object({eng.create_box_mesh(), mtl});
                }
                auto *bb = bboxes[id];
                bb->set_visible(it.second.visible);
                bb->set_transform(math::create_translation<float>(it.second.center));
                bb->set_scale(it.second.size);
            }
        }
        // render scene
        running = eng.render();

        if (socket_simcontrol_req) {
            advance_time(socket_simcontrol_req);
        }

        cnt++;
    }

    running = false;
    log_info("joining socket threads...");
    for (auto &t : threads) {
        t.join();
    }

    eng.terminate();
    return 0;
}

