#include <iostream>
#include <string>
#include <rados/librados.hpp>
#include <chrono>

int main(int argc, const char** argv) {
    int object_count=2000;
    
    int ret = 0;

    /* Declare the cluster handle and required variables. */
    librados::Rados cluster;
    char cluster_name[] = "ceph";
    char user_name[] = "client.admin";
    uint64_t flags = 0;

    /* Initialize the cluster handle with the "ceph" cluster name and "client.admin" user */
    {
        ret = cluster.init2(user_name, cluster_name, flags);
        if (ret < 0) {
            std::cerr << "Couldn't initialize the cluster handle! error " << ret << std::endl;
            return EXIT_FAILURE;
        } else {
            std::cout << "Created a cluster handle." << std::endl;
        }
    }

    /* Read a Ceph configuration file to configure the cluster handle. */
    {
        ret = cluster.conf_read_file("/etc/ceph/ceph.conf");
        if (ret < 0) {
            std::cerr << "Couldn't read the Ceph configuration file! error " << ret << std::endl;
            return EXIT_FAILURE;
        } else {
            std::cout << "Read the Ceph configuration file." << std::endl;
        }
    }

    /* Read command line arguments */
    {
        ret = cluster.conf_parse_argv(argc, argv);
        if (ret < 0) {
            std::cerr << "Couldn't parse command line options! error " << ret << std::endl;
            return EXIT_FAILURE;
        } else {
            std::cout << "Parsed command line options." << std::endl;
        }
    }

    /* Connect to the cluster */
    {
        ret = cluster.connect();
        if (ret < 0) {
            std::cerr << "Couldn't connect to cluster! error " << ret << std::endl;
            return EXIT_FAILURE;
        } else {
            std::cout << "Connected to the cluster." << std::endl;
        }
    }

    librados::IoCtx io_ctx;
    const char* pool_name = "mypool";

    {
        ret = cluster.ioctx_create(pool_name, io_ctx);
        if (ret < 0) {
            std::cerr << "Couldn't set up ioctx! error " << ret << std::endl;
            exit(EXIT_FAILURE);
        } else {
            std::cout << "Created an ioctx for the pool." << std::endl;
        }
    }

    // int blockSizeArr[] = { 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288,  1048576,  2097152 ,4194304 ,8388608 , 16777216 };
    int blockSizeArr[11] = { 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288,  1048576,  2097152 ,4194304 };

    for(int i=0; i<11; i++){
        int object_size = blockSizeArr[i];

        std::cout << std::endl;
        std::cout<<object_size<<std::endl;

        int64_t write[object_count];
        int64_t read[object_count];
        int64_t del[object_count];
        
        for(int j=0; j<object_count; j++){
            librados::bufferlist bl;
            bl.clear();

            char data[j];
            std::string object_name = "bench"+std::to_string(j);

            /* Write an object synchronously. */
            {
                bl.append(data);

                auto start = std::chrono::high_resolution_clock::now();
                ret = io_ctx.write_full(object_name, bl);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

                if (ret < 0) {
                    std::cerr << "Couldn't write object! error " << ret << std::endl;
                    exit(EXIT_FAILURE);
                } else{
                    write[j] = duration.count();
                }
            }
        }

        for (int j = 0; j < object_count; j++) {
            librados::bufferlist bl;
            bl.clear();

            char data[j];
            std::string object_name = "bench" + std::to_string(j);

            /*
             * Read the object back synchronously.
             */
            {
                auto start = std::chrono::high_resolution_clock::now();
                ret = io_ctx.read(object_name, bl, object_size, 0);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

                if (ret < 0) {
                    std::cerr << "Couldn't start read object! error " << ret << std::endl;
                    exit(EXIT_FAILURE);
                } else {
                    read[j] = duration.count();
                }
            }
        }

        for (int j = 0; j < object_count; j++) {
            std::string object_name = "bench" + std::to_string(j);

            /*
             * Remove the object.
             */
            {
                auto start = std::chrono::high_resolution_clock::now();
                ret = io_ctx.remove(object_name);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

                if (ret < 0) {
                    std::cerr << "Couldn't remove object! error " << ret << std::endl;
                    exit(EXIT_FAILURE);
                } else {
                    del[j] = duration.count();
                }
            }
        }

        int64_t writeTotal = 0;
        int64_t readTotal = 0;
        int64_t delTotal = 0;

        for(int j=0; j<object_count; j++){
            writeTotal += write[j];
            readTotal += read[j];
            delTotal += del[j];
        }

        std::cout << "Write avg = " << float(writeTotal) / float(object_count *1000) << std::endl;
        std::cout << "Read avg = " << float(readTotal) / float(object_count *1000) << std::endl;
        std::cout << "Delete avg = " << float(delTotal) / float(object_count*1000) << std::endl;
        std::cout<<std::endl;
    }

    io_ctx.close();
    cluster.shutdown();

    return 0;
}