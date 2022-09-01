
"
"
========================================================================================
                            Create Chameleon Trovi (1 node)
========================================================================================

0. Guideline 
    # https://chameleoncloud.readthedocs.io/en/latest/technical/sharing.html
    # BEST: https://chameleoncloud.readthedocs.io/en/latest/technical/sharing.html#packaging-shared-artifacts
        # Forking
            # click on the “Share” tab and select “Package as a new artifact”. 

1. Homepage
    # Visit: https://www.chameleoncloud.org/experiment/share/
    # Open one of the existing trovi, e.g. FIO test 
    # Open Hub Control Panel 
        # https://jupyter.chameleoncloud.org/user/daniar.h.k@gmail.com/lab?redirects=1
        # Click "Start my Server" 
            # Server name       URL                                         Last activity           Actions
            # test-cortx-motr   /user/daniar.h.k@gmail.com/test-cortx-motr  a few seconds ago   

        # CLick the URL!!
        # You will see these:
            notebooks/
            Welcome.ipynb

    # Create your own ipynb!
        # Click the "+" sign, color blue left upper corner
        # Click "Notebook" ==> "Bash"
            # Don't pick "Python 3 (ipykernel)" because the cell will be a python console
        # Rename the file "CORTX-Motr.ipynb"

    # Create a folder "CORTX-Motr/"
        # Drag the CORTX-Motr.ipynb into this folder
        # We need this folder so that we can click "Share" -> "Package as a new artifact"
            # Otherwise, the "Package as a new artifact" will be grayed out (unclickable)

    # Start adding columns/cell 
        # Two types: "Code" or "Markdown"

    # Publish the artifact 
        # Click the folder "CORTX-Motr/"
        # Click "Share" -> "Package as a new artifact"

2. BUGS/ERRORs 
    
    # Calling "openstack keypair list "
        # https://jupyter.chameleoncloud.org/user/daniar.h.k@gmail.com/test-cortx-motr/lab/workspaces/auto-G/tree/notebooks/tutorials/getting-started/JupyterOrchestration.ipynb
        # got: Unexpected exception for http://:5000/v3/OS-FEDERATION/identity_providers/chameleon/protocols/openid/auth: Invalid URL 'http://:5000/v3/OS-FEDERATION/identity_providers/chameleon/protocols/openid/auth': No host supplied


"
"
========================================================================================
                        Run a Single Node CORTX-Motr (Loops)
========================================================================================

# project ..849 @UC "Skylake"
    # @TACC is better, faster network speed 
# based on Faradawn guides:
    https://github.com/faradawn/tutorials/blob/main/linux/cortx/motr_guide.md

# Create Reservation
    # https://chi.tacc.chameleoncloud.org/project/
    
    1. Reserve Physical Host
        # click "Leases" => "+ Create Lease"
        # lease name = "dan-cortx"
            "$node_type", "compute_skylake"
            Max lease: 
                7 days
    2. Reserve Network [No-NEED]

# Launching an Instance
    # In the sidebar, click Compute, then click Instances
    # Click on the Launch Instance
        # pick the correct reservation 
        # count = 1 (for singlenode)
        # Image: "CC-CentOS7"    # need 1062?? no
    # Select "sharednet1"
    # Choose the ssh key
        # "dan-macpro"

# Allocate floating IPs
    # Book the IP interface
        # Click "Network -> Floating IPs -> Allocate IP To Project"
        # Write description
        # Click "Allocate IP"

    # click "Associate" OR click "attach interface"
        # Click "Network -> Floating IPs"

    # ony 6 available public interfaces
    # wait a few minutes
    # is it Spawning?? https://chi.tacc.chameleoncloud.org/project/instances/

# If you can't access the cc user on SSH
    # open the console terminal via the website
    # edit the .ssh/authorized_keys and add your pub_key manually!!!
    # now, the hostname will be different 
        # e.g. from dan-storage to zhenz-test
    # run "sudo dhclient" from the web-based ssh console

00. Preparation [Login as "cc"]
    # Use cc user!!
    ssh cc@129.114.108.8
    # Setup disk 
        # check if there is already mounted disk
        df -H
            # /dev/sda1       251G  2.8G  248G   2% /
            # should be enough

    # Setup user daniar 
    sudo adduser daniar
    sudo usermod -aG wheel daniar
    sudo su 
    cp -r /home/cc/.ssh /home/daniar
    chmod 700  /home/daniar/.ssh
    chmod 644  /home/daniar/.ssh/authorized_keys
    chown daniar  /home/daniar/.ssh
    chown daniar  /home/daniar/.ssh/authorized_keys
    echo "daniar ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers.d/90-cloud-init-users
    exit
    exit


0. Setup zsh [Login on "daniar"]

    ssh 129.114.108.8 

        sudo su
        yum update -y
        yum install zsh -y
        chsh -s /bin/zsh root

        # Break the Copy here ====

        exit
        sudo chsh -s /bin/zsh daniar
        which zsh
        echo $SHELL

        sudo yum install wget git vim zsh -y
 
        # Break the Copy here ====

        printf 'Y' | sh -c "$(wget -O- https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

        /bin/cp ~/.oh-my-zsh/templates/zshrc.zsh-template ~/.zshrc
        sudo sed -i 's|home/daniar:/bin/bash|home/daniar:/bin/zsh|g' /etc/passwd
        sudo sed -i 's|ZSH_THEME="robbyrussell"|ZSH_THEME="risto"|g' ~/.zshrc
        zsh
        exit
        exit

1. Setup passwordless SSH among servers [No-NEED]
    # Go ahead if you guys want to run this as well
    
    # Run at Local
        declare -a arrIP=("192.5.87.198")
        for ip in "${arrIP[@]}"
        do
            echo "$ip"
            scp /Users/daniar/Documents/_CORTX/ssh-key/id_rsa $ip:~/.ssh/
            scp /Users/daniar/Documents/_CORTX/ssh-key/id_rsa.pub $ip:~/.ssh/
            ssh $ip "cd ~/.ssh/; cat id_rsa.pub >> authorized_keys; echo 'StrictHostKeyChecking no' > config; chmod go-rw config"
        done

2. Clone Motr and Install dependencies
    # clone repository

    sudo mkdir -p /mnt/extra
    sudo chown daniar -R /mnt

    cd /mnt/extra
    git clone --recursive https://github.com/Seagate/cortx-motr.git

    # install pip and python
    yum group install -y "Development Tools"
    yum install -y python-devel ansible tmux
    curl https://bootstrap.pypa.io/pip/2.7/get-pip.py -o get-pip.py
    python get-pip.py pip==19.3.1            
    sudo pip install --target=/usr/lib64/python2.7/site-packages ipaddress

    # force ansible to use python2
    sudo su
    echo "all:" >> /etc/ansible/hosts
    echo "  ansible_python_interpreter: \"/usr/bin/python2\"" >> /etc/ansible/hosts
    exit

    # run build dependencies (9 min)
    cd /mnt/extra/cortx-motr
    sudo ./scripts/install-build-deps

        # PLAY RECAP **************************************************************************************
        # localhost                  : ok=79   changed=49   unreachable=0    failed=0    skipped=12   rescued=0    ignored=1
        # make sure that failed=0!!!

3. Configure libfabric 
    
    cd /mnt/extra
    wget https://github.com/Seagate/cortx/releases/download/build-dependencies/libfabric-1.11.2-1.el7.x86_64.rpm
    wget https://github.com/Seagate/cortx/releases/download/build-dependencies/libfabric-devel-1.11.2-1.el7.x86_64.rpm
    sudo rpm -i libfabric-1.11.2-1.el7.x86_64.rpm
    sudo rpm -i libfabric-devel-1.11.2-1.el7.x86_64.rpm
    sudo sed -i 's|tcp(eth1)|tcp(eth0)|g' /etc/libfab.conf

4. Build motr 
    # Make sure that the exampleX.c is in the ORIGINAL form, otherwise, you can't run "make"
    # (7 min)
    cd /mnt/extra/cortx-motr
    sudo ./autogen.sh

    # Disable expensive checks 
        # https://cortxcommunity.slack.com/archives/C019S0SGWNQ/p1607974535256000
        
        # sudo ./configure --disable-expensive-checks --disable-immediate-trace  --disable-dev-mode --with-trace-max-level=M0_ERROR

        # for bottleneck analysis
        sudo ./configure --with-trace-max-level=M0_DEBUG
        # ./configure --help | grep trace
            #   --with-trace-max-level=<M0_ALWAYS|M0_FATAL|M0_ERROR|M0_WARN|M0_NOTICE|M0_INFO|M0_DEBUG>
            #      set highest trace level that will be enabled, all
            #      levels with verbosity above that will not be logged
        # LOG_LEVEL: 4               # err(0), warn(1), info(2), trace(3), debug(4)
            # /mnt/extra/cortx-motr/dtm0/it/all2all/m0crate.yaml.in


    cd /mnt/extra/cortx-motr
    sudo make -j48

5. Compile Python util
    
    cd /mnt/extra/
    sudo yum install -y gcc rpm-build python36 python36-pip python36-devel python36-setuptools openssl-devel libffi-devel python36-dbus
    git clone --recursive https://github.com/Seagate/cortx-utils -b main

    cd /mnt/extra/cortx-utils
    ./jenkins/build.sh -v 2.0.0 -b 2
    sudo pip3 install -r https://raw.githubusercontent.com/Seagate/cortx-utils/main/py-utils/python_requirements.txt
    sudo pip3 install -r https://raw.githubusercontent.com/Seagate/cortx-utils/main/py-utils/python_requirements.ext.txt
    
    cd /mnt/extra/cortx-utils/py-utils/dist
    sudo yum install -y cortx-py-utils-*.noarch.rpm
    # sudo find . -type f -name '*noarch.rpm'

6. Building Hare 
    
    cd /mnt/extra
    # clone repo
    git clone https://github.com/Seagate/cortx-hare.git && cd cortx-hare

    # install fecter
    sudo yum -y install python3 python3-devel yum-utils
    sudo yum localinstall -y https://yum.puppetlabs.com/puppet/el/7/x86_64/puppet-agent-7.0.0-1.el7.x86_64.rpm
    sudo ln -sf /opt/puppetlabs/bin/facter /usr/bin/facter

    # install consul
    sudo yum -y install yum-utils
    sudo yum-config-manager --add-repo https://rpm.releases.hashicorp.com/RHEL/hashicorp.repo
    sudo yum -y install consul-1.9.1

    # build and install motr [here
    cd /mnt/extra/cortx-motr && time sudo ./scripts/install-motr-service --link
    export M0_SRC_DIR=$PWD

    # build hare 
    cd /mnt/extra/cortx-hare
    sudo make
    sudo make install

    # create hare group
    sudo groupadd --force hare
    sudo usermod --append --groups hare $USER
    sudo su

    # add path to zsh
    echo 'PATH=/opt/seagate/cortx/hare/bin:$PATH' >> ~/.zshrc
    echo 'export LD_LIBRARY_PATH=/mnt/extra/cortx-motr/motr/.libs/' >> ~/.zshrc
    source ~/.zshrc
    
8. Create loop devices [[AFTER EACH REBOOT]]
    # https://www.thegeekdiary.com/how-to-create-virtual-block-device-loop-device-filesystem-in-linux/
    # If there is only 1 physical storage, you must create loop devices!
        # linux support block device called the loop device, which maps a normal file onto a virtual block device

    # Create a file (25 GB each)

        mkdir -p /mnt/extra/loop-files/
        cd /mnt/extra/loop-files/
        dd if=/dev/zero of=loopbackfile1.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile2.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile3.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile4.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile5.img bs=100M count=250
        
        # check size 
        # du -sh loopbackfile1.img 

            # 1048576000 bytes (1.0 GB) copied, 0.723784 s, 1.4 GB/s
            # 1001M loopbackfile1.img

    # Create the loop device
        cd /mnt/extra/loop-files/
        sudo losetup -fP loopbackfile1.img
        sudo losetup -fP loopbackfile2.img
        sudo losetup -fP loopbackfile3.img
        sudo losetup -fP loopbackfile4.img
        sudo losetup -fP loopbackfile5.img

    # print loop devices 
        losetup -a
            # /dev/loop0: []: (/mnt/extra/loop-files/loopbackfile1.img)
            # /dev/loop1: []: (/mnt/extra/loop-files/loopbackfile2.img)
            # /dev/loop2: []: (/mnt/extra/loop-files/loopbackfile3.img)

    # Format devices into filesystems 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile1.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile2.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile3.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile4.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile5.img 

    # mount loop devices

        mkdir -p /mnt/extra/loop-devs/loop0
        mkdir -p /mnt/extra/loop-devs/loop1
        mkdir -p /mnt/extra/loop-devs/loop2
        mkdir -p /mnt/extra/loop-devs/loop3
        mkdir -p /mnt/extra/loop-devs/loop4
        cd /mnt/extra/loop-devs/
        sudo mount -o loop /dev/loop0 /mnt/extra/loop-devs/loop0
        sudo mount -o loop /dev/loop1 /mnt/extra/loop-devs/loop1
        sudo mount -o loop /dev/loop2 /mnt/extra/loop-devs/loop2
        sudo mount -o loop /dev/loop3 /mnt/extra/loop-devs/loop3
        sudo mount -o loop /dev/loop4 /mnt/extra/loop-devs/loop4
        lsblk -f
        df -h 

        # remove loop devs [No-NEED]
            # sudo umount /mnt/extra/loop-devs/loop0
            # sudo umount /mnt/extra/loop-devs/loop1
            # sudo umount /mnt/extra/loop-devs/loop2
            # sudo umount /mnt/extra/loop-devs/loop3
            # sudo umount /mnt/extra/loop-devs/loop4
            # sudo losetup -d /dev/loop0
            # sudo losetup -d /dev/loop1
            # sudo losetup -d /dev/loop2
            # sudo losetup -d /dev/loop3
            # sudo losetup -d /dev/loop4
            # rm -rf /mnt/extra/loop-files/*.img

        # check using "lsblk"
            # we will use "loop5" "loop6" "loop7" for Motr 
            # "loop8" for log 

8. ACTIVATE the lnet [AFTER EACH REBOOT]
    
    # "FIND interface for LNET"
    sudo ifconfig | grep MULTICAST | grep RUNNING
    # use the ethXX address to initiate lnetctl

    # configure Luster (use eth0 which is UP)
    sudo sed -i 's|tcp(eth1)|tcp(eth0)|g' /etc/modprobe.d/lnet.conf
    # sudo sed -i 's|tcp(eth0)|tcp(eth1)|g' /etc/modprobe.d/lnet.conf
    cat /etc/modprobe.d/lnet.conf
    sudo modprobe lnet

    # sudo systemctl stop lnet
    # sudo systemctl start lnet
    # sudo lnetctl net add --net tcp --if eth0

    sudo lctl list_nids

        # sample output: 10.52.0.167@tcp

9. Start a Hare cluster

    # Prepare cdf file 
        cd /mnt/extra
        cp /opt/seagate/cortx/hare/share/cfgen/examples/singlenode.yaml CDF.yaml

        sudo sed -i "s|hostname: localhost|hostname: `hostname`|g" CDF.yaml
        sudo sed -i "s|node: localhost|node: `hostname`|g" CDF.yaml
        sudo sed -i 's|data_iface: eth1|data_iface: eth0|g' CDF.yaml
        # sudo sed -i 's|data_iface: eth0|data_iface: eth1|g' CDF.yaml

        # remove the unavailable storage devices 
        sed -i '/loop0/d' CDF.yaml
        sed -i '/loop1/d' CDF.yaml
        sed -i '/loop2/d' CDF.yaml
        sed -i '/loop3/d' CDF.yaml
        sed -i '/loop4/d' CDF.yaml
        sed -i '/loop8/d' CDF.yaml

        # set the disk for logging
        sudo sed -i "s|loop9|loop8|g" CDF.yaml

        # Check the modification
            cd /mnt/extra
            cat CDF.yaml| grep eth
            cat CDF.yaml| grep hostname
            cat CDF.yaml| grep node:            # only 3 loop devices 

            # Sample final CDF:
                #nodes:
                #  - hostname: node-1.novalocal     # [user@]hostname
                #    node_group: localhost
                #    data_iface: eth0        # name of data network interface
                #    data_iface_ip_addr: null
                #    transport_type: libfab
                #    m0_servers:
                #      - runs_confd: true
                #        io_disks:
                #          data: []
                #          log: []
                #          meta_data: null
                #      - runs_confd: null
                #        io_disks:
                #          data:
                #            - path: /dev/loop5
                #            - path: /dev/loop6
                #            - path: /dev/loop7
                #          log:
                #            - path: /dev/loop8
                #          meta_data: null
                #    m0_clients:
                #      - name: m0_client_other  # name of the motr client
                #        instances: 2   # Number of instances, this host will run
                #create_aux: false # optional; supported values: "false" (default), "true"
                #pools:
                #  - name: the pool
                #    type: sns  # optional; supported values: "sns" (default), "dix", "md"
                #    disk_refs:
                #      - { path: /dev/loop5, node: node-1.novalocal }
                #      - { path: /dev/loop6, node: node-1.novalocal }
                #      - { path: /dev/loop7, node: node-1.novalocal }
                #    data_units: 1
                #    parity_units: 0
                #    spare_units: 0

    # bootstrap (0.5 min)
        cd /mnt/extra
        sudo hctl bootstrap --mkfs CDF.yaml
        hctl status
        
        # cd /mnt/extra; sudo hctl shutdown; sudo hctl bootstrap --mkfs CDF.yaml

    # check status
        hctl status

        # sample output:
            Data pool:
                # fid name
                0x6f00000000000001:0x0 'the pool'
            Profile:
                # fid name: pool(s)
                0x7000000000000001:0x0 'default': 'the pool' None None
            Services:
                dan-cortx.novalocal  (RC)
                [started]  hax                 0x7200000000000001:0x0          inet:tcp:10.52.0.167@22001
                [started]  confd               0x7200000000000001:0x1          inet:tcp:10.52.0.167@21002
                [started]  ioservice           0x7200000000000001:0x2          inet:tcp:10.52.0.167@21003
                [unknown]  m0_client_other     0x7200000000000001:0x3          inet:tcp:10.52.0.167@22501
                [unknown]  m0_client_other     0x7200000000000001:0x4          inet:tcp:10.52.0.167@22502

10. Connect VSCode ssh 
    # Use the "remote explorer" package to establish remote editor
    # Open the /mnt/extra 
    # IMPORTANT: The #12 Clear cache loop must be run via VSCode!
    # Need this to inspect the CDF file, make sure it looks good
    # Also for modifying the motr source code

11. Clone CORTX-bench-extra
    # Change the "daniarherikurniawan" with your github username
    # Set passwordless github push/pull 
    git config --global user.email "ddhhkk2@gmail.com"
    git config --global user.name "daniarherikurniawan"
    git config --global credential.helper store         # store pass in ~/.git-credentials as plain text format.

    cd /mnt/extra 
    git clone https://daniarherikurniawan@github.com/daniarherikurniawan/cortx-bench-extra.git
    
    # Break the Copy here ====

    cd /mnt/extra/
    cp cortx-bench-extra/motr-clients/*  cortx-motr/motr/examples/      # copy Motr sample client 
    cp -r cortx-bench-extra/script  cortx-motr/motr/examples/

12. Run Clear Cache loop  
    # Run this in the VSCode terminal, otherwise the process will terminate easily
    # Background Task [On other terminal]
    # keep this script running while benchmarking 

    cd /mnt/extra/cortx-motr/motr/examples/script/
    sudo ./free_page_cache.sh 0.25

13. Running Client 
    
    # example1.c [default client]
        cd /mnt/extra/cortx-motr/motr/examples
        gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr example1.c -o example1
        ./example1 inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670
        #                                                                         mc_profile              mc_process_fid

    # example1_dan.c 

        # Single benchmark 

            #!/usr/bin/env bash

            cd /mnt/extra/cortx-motr/motr/examples
            ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_dan.c -params "N_REQUEST=200 BLOCK_SIZE=32768"
            gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr example1_dan.c -o example1_dan
            ./example1_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670

            # delete the m0trace
            rm -rf m0trace*

        # Benchmark

            cd /mnt/extra/cortx-motr/motr/examples
            blockSizeArr=("4194304" "8388608" "16777216")
            for blockSize in "${blockSizeArr[@]}"
            do
                echo "$blockSize"
                ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_dan.c -params "N_REQUEST=20000 BLOCK_SIZE=$blockSize"
                gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr example1_dan.c -o example1_dan
                ./example1_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670
                rm -rf m0trace*     # don't run this line if you will use ADDB
            done


12. Run Multi Threaded Client [USE-THIS]
    
    # restart cluster if needed: cd /mnt/extra; sudo hctl shutdown; sudo hctl bootstrap --mkfs CDF.yaml

    # Write/Read/Delete
    #   1  /  1 /   1

    cd /mnt/extra/cortx-motr/motr/examples
    ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_multithd_dan.c -params "N_REQUEST=100 BLOCK_SIZE=1048576 N_PARALLEL_THD=1"
    gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr -lpthread example1_multithd_dan.c -o example1_multithd_dan

    ./example1_multithd_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 1 0 0 
    ./example1_multithd_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 1 0 
    ./example1_multithd_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 0 1 

    # Benchmark type-A
        # "4096" "8192" "16384" "32768" "65536" "131072" "262144" "524288" "1048576" "2097152" "4194304" "8388608" "16777216"

        cd /mnt/extra/cortx-motr/motr/examples
        blockSizeArr=("2097152" "4194304" "8388608" "16777216")
        for blockSize in "${blockSizeArr[@]}"
        do
            echo "$blockSize"
            ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_multithd_dan.c -params "N_REQUEST=20000 BLOCK_SIZE=$blockSize"
            gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr -lpthread example1_multithd_dan.c -o example1_multithd_dan
            ./example1_multithd_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 1 0 0 
            ./example1_multithd_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 1 0 
            ./example1_multithd_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 0 1 
            rm -rf m0trace*     # don't run this line if you will use ADDB
        done


    # Benchmark type-B
        cd /mnt/extra/cortx-motr/motr/examples
        blockSizeArr=("2097152" "4194304" "8388608" "16777216")
        for blockSize in "${blockSizeArr[@]}"
        do
            echo "$blockSize"
            ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_multithd_dan.c -params "N_REQUEST=20000 BLOCK_SIZE=$blockSize"
            gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr -lpthread example1_multithd_dan.c -o example1_multithd_dan
            ./example1_multithd_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 1 1 1
            rm -rf m0trace*     # don't run this line if you will use ADDB
        done

"
"
========================================================================================
                            Run ADDB on M0trace output
========================================================================================
# Faradawn Notes: https://docs.google.com/document/d/1YVZsuJtXyGPnhZ66C-cdzSopaLo_WHjgyb74-BZ-FWM/edit
# Keywords to search on Slack: "m0trace", "m0addb2dump", "addb"
    # slack: 
        # Philippe Daniel + Andriy + Anatoliy : https://cortxcommunity.slack.com
            # Anatoliy: "CORTX Observability with Anatoily Bilenko" https://www.youtube.com/watch?v=FFTi2XNFb7A
                # Good for paper intro of addb 
    # addb2 primer: https://github.com/Seagate/cortx-motr/blob/main/doc/addb2-primer
    # happy-path demo: https://github.com/Seagate/cortx-motr/blob/documentation/doc/dev/dtm/dtm0-demo-happy-path.org#integration-test
    # github:
        # https://github.com.org#integration-test
            # Integration test -> integrate addb stobs

0. Preparation
    # Follow Motr deployment tutorial (e.g "Run a Single Node CORTX-Motr (Loops)")
    # Run the client 
        # Get the m0trace.* output "m0trace.392531.2022-07-26-17:21:51"

    # Create the loop device  [Run AFTER EACH REBOOT]
        cd /mnt/extra/loop-files/
        sudo losetup -fP loopbackfile1.img
        sudo losetup -fP loopbackfile2.img
        sudo losetup -fP loopbackfile3.img
        sudo losetup -fP loopbackfile4.img
        sudo losetup -fP loopbackfile5.img

    # print loop devices  [Run AFTER EACH REBOOT]
        losetup -a
            # /dev/loop0: []: (/mnt/extra/loop-files/loopbackfile1.img)
            # /dev/loop1: []: (/mnt/extra/loop-files/loopbackfile2.img)
            # /dev/loop2: []: (/mnt/extra/loop-files/loopbackfile3.img)

    # Format devices into filesystems  [Run AFTER EACH REBOOT]
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile1.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile2.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile3.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile4.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile5.img 
        lsblk -f

    # mount loop devices [Run AFTER EACH REBOOT]

        mkdir -p /mnt/extra/loop-devs/loop0
        mkdir -p /mnt/extra/loop-devs/loop1
        mkdir -p /mnt/extra/loop-devs/loop2
        mkdir -p /mnt/extra/loop-devs/loop3
        mkdir -p /mnt/extra/loop-devs/loop4
        cd /mnt/extra/loop-devs/
        sudo mount -o loop /dev/loop0 /mnt/extra/loop-devs/loop0
        sudo mount -o loop /dev/loop1 /mnt/extra/loop-devs/loop1
        sudo mount -o loop /dev/loop2 /mnt/extra/loop-devs/loop2
        sudo mount -o loop /dev/loop3 /mnt/extra/loop-devs/loop3
        sudo mount -o loop /dev/loop4 /mnt/extra/loop-devs/loop4
        lsblk -f
        df -h 

    # START Motr Cluster 
        cd /mnt/extra; sudo hctl shutdown; sudo hctl bootstrap --mkfs CDF.yaml; hctl status


1. Enable ADDB 
    # set configure motr to allow all logs (INFO) level 
    # recompile motr  
    # Put in the client.c: "motr_conf.mc_is_addb_init     = true;"     # Enable ADDB
    # recompile client 
    # restart hare cluster 
    # then run client and make sure you have the m0trace.* files 

2. Run Client: example1_dan.c 

    #!/usr/bin/env bash

    cd /mnt/extra/cortx-motr/motr/examples
    ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_dan.c -params "N_REQUEST=200 BLOCK_SIZE=32768"
    gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr example1_dan.c -o example1_dan
    ./example1_dan inet:tcp:10.52.0.167@22001 inet:tcp:10.52.0.167@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670

    # Make sure the addb is enabled ("motr_conf.mc_is_addb_init     = true")
        # Output: 
            addb_215065/
            m0trace.215065.2022-08-10-17:14:13

3. Process ADDB Trace  

    # install dependencies 
        pip3 install pandas 
        pip3 install graphviz peewee 
        pip3 install numpy tqdm plumbum graphviz
        sudo yum install -y python-pydot python-pydot-ng graphviz

        cd /mnt/extra/cortx-motr/motr/examples/
        git clone --recursive https://github.com/Seagate/seagate-tools
        ln -s seagate-tools/performance/PerfLine/roles/perfline_setup/files/chronometry chronometry
            # the chronometry is now at "/mnt/extra/chronometry"

    filename="m0trace.215065.2022-08-10-17:14:13"
    
    # Run "m0trace"
        # Flatten out the m0trace raw file 

        cd /mnt/extra/cortx-motr/motr/examples/
        sudo /mnt/extra/cortx-motr/utils/trace/m0trace -i $filename > $filename.log
          
    # Run "m0tracedump"
        # Get the dump file 

        cd /mnt/extra/cortx-motr/motr/examples/
        sudo /mnt/extra/cortx-motr/utils/m0tracedump -i m0trace.194241.2022-08-10-16:58:02 > $filename.dump

    # Run "m0addb2dump"
        # will generate stobs to be inserted to DB 
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        sudo /mnt/extra/cortx-motr/utils/m0addb2dump -f --  `pwd`/$addb_folder/o/100000000000000\:2 >  $addb_folder.addb2dump
            # output: addb_215065.addb2dump
        
    # Generate the DB object ("addb2db.py")
        # use seagate-tools repo script to generate m0play.db
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        python3 /mnt/extra/chronometry/addb2db.py --dumps $addb_folder.addb2dump --db seagate_tools_$addb_folder.db
            # output: "seagate_tools_addb_215065.db" ==> The most IMPORTANT file

        # Query the DB 
            cd /mnt/extra/cortx-motr/motr/examples/
            addb_folder="addb_215065"
            python3 /mnt/extra/chronometry/req_timelines.py -d seagate_tools_$addb_folder.db -p 215065 1777 --output-file req_timelines_215065_1777.png

                # Output: 
                    # Process pid 215065, id 1777
                    # 215065_1777 -> -5306630172570288004_184 (rpc_to_sxid);
                    # -5306630172570288004_184 -> 215065_1778 (sxid_to_rpc);

    # Open Sqlite Console 
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        sqlite3 seagate_tools_$addb_folder.db

        # List all tables 
            .tables

                # attr            host            relation        request         s3_request_uid

                # SELECT * FROM request LIMIT 10;
                # SELECT * FROM attr LIMIT 10;
                # SELECT * FROM s3_request_uid LIMIT 10;    # EMPTY because we are not using S3 here 

        # Get columns in a table 
            # table name is "relation"

            .schema relation
            PRAGMA table_info(attr);
                # |entity_id|pid|name|val|

            PRAGMA table_info(host);
                # |pid|hostname|

            PRAGMA table_info(request);
                # |time|pid|id|state|type_id|

            PRAGMA table_info(s3_request_uid);
                # |pid|id|uuid|

            PRAGMA table_info(relation);
                # |pid1|mid1|pid2|mid2|type_id|

        # List all types of relation 

            SELECT DISTINCT type_id FROM relation;
                # sxid_to_rpc
                # rpc_to_fom
                # rpc_to_sxid
                # client_to_ioo
                # bulk_to_rpc
                # ioo_to_rpc
                # cob_to_rpc
                # dix_to_cas
                # cas_to_rpc
                # client_to_cob

            SELECT count(type_id) FROM relation WHERE type_id="sxid_to_rpc";
                /* 2104 */
            SELECT count(type_id) FROM relation WHERE type_id="rpc_to_fom";
            SELECT count(type_id) FROM relation WHERE type_id="rpc_to_sxid";
            SELECT count(type_id) FROM relation WHERE type_id="client_to_ioo";
                /* 400 */
            SELECT count(type_id) FROM relation WHERE type_id="bulk_to_rpc";
                /* 400 */
            SELECT count(type_id) FROM relation WHERE type_id="ioo_to_rpc";
                /* 400 */
            SELECT count(type_id) FROM relation WHERE type_id="cob_to_rpc";
                /* 1600 */
            SELECT count(type_id) FROM relation WHERE type_id="dix_to_cas";
            SELECT count(type_id) FROM relation WHERE type_id="cas_to_rpc";
            SELECT count(type_id) FROM relation WHERE type_id="client_to_cob";

            # Find out more about sxid_to_rpc
                SELECT * FROM relation WHERE type_id="sxid_to_rpc" LIMIT 10;

                    # python3 /mnt/extra/chronometry/req_graph.py -d seagate_tools_$addb_folder.db -p 0 1


        SELECT * FROM relation LIMIT 10;
                # 215065|6739|215065|6740|client_to_ioo
                # 215065|6742|-5306630172570288004|1425|rpc_to_sxid
                # 215065|6741|215065|6742|bulk_to_rpc
                # 215065|6740|215065|6742|ioo_to_rpc
                    # Note "6739" will give the best graph!!
                    # it will have dependencies to another graphs!!

        SELECT * FROM relation where type_id="sxid_to_rpc" limit 10;
        SELECT * FROM relation where type_id="sxid_to_rpc" limit 10;

                # 215065|0|215065|0|rpc_to_fom
                # 215065|0|215065|0|rpc_to_fom
                # 215065|0|215065|0|rpc_to_fom

            # To quit, press: "Ctrl + D"

    # Generate Flow Graphs
        # https://www.youtube.com/watch?v=FFTi2XNFb7A
        # https://cortxcommunity.slack.com/archives/C019S0SGWNQ/p1659592200692869

        # sudo find / -name 'io_req.py'
        # sudo find / -name 'req_graph.py'

        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        python3 /mnt/extra/chronometry/req_graph.py -d seagate_tools_$addb_folder.db -p 215065 0
            # Process pid 215065, id 0
            # 215065_0 -> 215065_0 (rpc_to_fom);

                # output:
                    # attr_graph_215065_0.png --> BUT this is empty

        python3 /mnt/extra/chronometry/req_graph.py -d seagate_tools_$addb_folder.db -p 215065 1777
                # attr_graph_215065_1777.png

        # Find interesting pid to graph 
            cd /mnt/extra/cortx-motr/motr/examples/
            addb_folder="addb_215065"

            # get all "cob_id"
            cat $addb_folder.addb2dump | grep "cob_id" > cob_id.txt
                # cat $addb_folder.addb2dump | grep "o--> initialised" | tail -n 10
                
                # get the list of id only put it in "cob.txt"

                # get the most occurence
                sed -e 's/\s/\n/g' < cob.txt | sort | uniq -c | sort -nr | head  -10        
                   # 8 9030 
                   # 8 8990
                   # 8 8950 -> use this 

                # graph the potential pid 
                    # use pid - 1, because we want to capture the prior action as well  
                    cd /mnt/extra/cortx-motr/motr/examples/
                    addb_folder="addb_215065"
                    python3 /mnt/extra/chronometry/req_graph.py -d seagate_tools_$addb_folder.db -p 215065 8949

    # Generate Timeline Diagram  
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        python3 /mnt/extra/chronometry/req_timelines.py -d seagate_tools_$addb_folder.db -p 215065 8949 --output-file req_timelines_215065_8949.png
            # output at req_timelines.png

    # Generate Histogram 
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        python3 /mnt/extra/chronometry/hist.py --db seagate_tools_$addb_folder.db   -t REQ_QUERY 1


    # Try running "queues.py"
        # based on /mnt/extra/chronometry
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        python3 /mnt/extra/chronometry/queues.py -d seagate_tools_$addb_folder.db

        # ERROR:
            # require "s3_request_state" table 

4. Download all the files/traces [Run at LOCAL]
    
    log_dir="addb-examples-v0"
    mkdir -p ~/Documents/_CORTX/$log_dir
    rsync -Pav daniar@129.114.108.103:/mnt/extra/cortx-motr/motr/examples ~/Documents/_CORTX/$log_dir
    # scp -rp daniar@129.114.108.103:/mnt/extra/cortx-motr/motr/examples ~/Documents/_CORTX/$log_dir


5. Generate the DB object using Motr tools [No-NEED]
    # Use the Motr tools
        # sudo find / -name 'addb2db.py'
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        python3 /mnt/extra/cortx-motr/scripts/addb-py/chronometry/addb2db.py --dumps $addb_folder.addb2dump --db motr_tools_$addb_folder.db


    # Open Sqlite Console 
        cd /mnt/extra/cortx-motr/motr/examples/
        addb_folder="addb_215065"
        sqlite3 motr_tools_$addb_folder.db

        # List all tables 
            echo ".table" | sqlite3 motr_tools_$addb_folder.db 
                # attr                  cob_to_rpc            queues
                # be_tx                 dix_req               rpc_req
                # bulk_to_rpc           dix_to_cas            rpc_to_sxid
                # cas_fom_to_crow_fom   dix_to_mdix           s3_measurement
                # cas_req               fom_desc              s3_request_state
                # cas_to_rpc            fom_req               s3_request_to_client
                # client_req            fom_req_state         s3_request_uid
                # client_to_cob         fom_to_stio           stio_req
                # client_to_dix         fom_to_tx             sxid_to_rpc
                # client_to_ioo         ioo_req               tx_to_gr
                # cob_req               ioo_to_rpc

            for table in `echo ".table" | sqlite3 motr_tools_$addb_folder.db`; do
                
                echo "$table \t" `echo "SELECT count(*) FROM $table;" | sqlite3 motr_tools_$addb_folder.db`
            done

                # attr     9197
                # cob_to_rpc   1600
                # queues   687
                # dix_req      4
                # rpc_req      11981
                # bulk_to_rpc      400
                # dix_to_cas   1
                # rpc_to_sxid      2073
                # cas_req      3
                # fom_desc     40
                # cas_to_rpc   1
                # fom_req      142
                # client_req   5600
                # fom_req_state    372
                # client_to_cob    1000
                # sxid_to_rpc      2104
                # client_to_ioo    400
                # ioo_req      1600
                # cob_req      3000
                # ioo_to_rpc   400


6. Using Graph script inside Motr Tools
    # at /mnt/extra/cortx-motr/scripts/addb-py/chronometry

    # Generate Timeline graph ("io_req.py")

        # https://youtu.be/FFTi2XNFb7A?t=1670
            cd /mnt/extra/cortx-motr/motr/examples/
            addb_folder="addb_215065"

            echo "SELECT DISTINCT type_id FROM relation;" | sqlite3 seagate_tools_$addb_folder.db 
            echo ".table" | sqlite3 seagate_tools_$addb_folder.db 

            python3 /mnt/extra/cortx-motr/scripts/addb-py/chronometry/io_req.py -d seagate_tools_$addb_folder.db -p 215065 8949

        # Error:
            # can't possibly run this because I don't have table "peewee.OperationalError: no such table: client_to_ioo"
            # Perhaps that table can be generated if we use S3 or RGW client 


    # Generate Timeline graph ("fom_req.py")

            # list all tables 
            echo ".table" | sqlite3 seagate_tools_$addb_folder.db 
                # attr            host            relation        request         s3_request_uid

            cd /mnt/extra/cortx-motr/motr/examples/
            addb_folder="addb_215065"
            python3 /mnt/extra/cortx-motr/scripts/addb-py/chronometry/fom_req.py -d seagate_tools_$addb_folder.db -p 215065 -f 3

        # Error:
            # "peewee.OperationalError: no such table: fom_desc"
            # can't possibly run fom_timeline.py as well 


    # Possible graph scripts 
        # It's impossible to run for now, because we don't have the required tables!

        # at /mnt/extra/cortx-motr/scripts/addb-py/chronometry
            hist__client_req.py
            hist__fom_req.py
            hist__fom_req_r.py
            hist__fom_to_rpc.py
            hist__ioo_req.py
            hist.py
            hist__s3req.py
            hist__srpc_to_crpc.py
            hist__stio_req.py
            ios-hist.sh


    # Attempting to run "hist__srpc_to_crpc.py"

        SELECT * FROM relation WHERE type_id="rpc_to_sxid" LIMIT 10;
        SELECT * FROM relation WHERE type_id="sxid_to_rpc" LIMIT 10;
            # 0|-1|215065|23|sxid_to_rpc
            # 0|1|215065|30|sxid_to_rpc
            # 0|2|215065|35|sxid_to_rpc
            # What is the meaning of these columns?????

        # I believe we should have "rpc_to_sxid" and "sxid_to_rpc" table!!
            # those tables will be based on "relation" table

        # the script contains: 
            SELECT (crq.time-srq.time), crq.state, srq.state

            FROM rpc_to_sxid
            JOIN sxid_to_rpc on rpc_to_sxid.xid=sxid_to_rpc.xid AND rpc_to_sxid.session_id=sxid_to_rpc.session_id
            JOIN rpc_req crq on rpc_to_sxid.id=crq.id AND rpc_to_sxid.pid=crq.pid
            JOIN rpc_req srq on sxid_to_rpc.id=srq.id AND sxid_to_rpc.pid=srq.pid

            # I believe the field/columns of table "rpc_to_sxid" and "sxid_to_rpc" should be defined somewhere!!!
                # otherwise, nothing we can do with the current data 

    # More issues
        # We don't have "request-browser.sh", Anatoliy has it when we present here: https://www.youtube.com/watch?v=FFTi2XNFb7A



"
"
========================================================================================
                        Run a Single Node CORTX-K8 (Loops)
========================================================================================

# project ..849 @UC "Skylake"
# based on Faradawn guides:
    https://github.com/faradawn/tutorials/blob/main/linux/cortx/README.md

# Create Reservation
    # https://chi.tacc.chameleoncloud.org/project/
    
    1. Reserve Physical Host
        # click "Leases" => "+ Create Lease"
        # lease name = "dan-cortx"
            "$node_type", "compute_skylake"
            Max lease: 
                7 days
    2. Reserve Network [No-NEED]

# Launching an Instance
    # In the sidebar, click Compute, then click Instances
    # Click on the Launch Instance
        # pick the correct reservation 
        # count = 1 (for singlenode)
        # Image: "CC-CentOS7"    # need 1062?? no
    # Select "sharednet1"
    # Choose the ssh key
        # "dan-macpro"

# Allocate floating IPs
    # Book the IP interface
        # Click "Network -> Floating IPs -> Allocate IP To Project"
        # Write description
        # Click "Allocate IP"

    # click "Associate" OR click "attach interface"
        # Click "Network -> Floating IPs"

    # ony 6 available public interfaces
    # wait a few minutes
    # is it Spawning?? https://chi.tacc.chameleoncloud.org/project/instances/

# If you can't access the cc user on SSH
    # open the console terminal via the website
    # edit the .ssh/authorized_keys and add your pub_key manually!!!
    # now, the hostname will be different 
        # e.g. from dan-storage to zhenz-test
    # run "sudo dhclient" from the web-based ssh console

00. Preparation [Login as "cc"]
    # Use cc user!!
    ssh cc@129.114.108.8
    # Setup disk 
        # check if there is already mounted disk
        df -H
            # /dev/sda1       251G  2.8G  248G   2% /
            # should be enough

    # Setup user daniar 
    sudo adduser daniar
    sudo usermod -aG wheel daniar
    sudo su 
    cp -r /home/cc/.ssh /home/daniar
    chmod 700  /home/daniar/.ssh
    chmod 644  /home/daniar/.ssh/authorized_keys
    chown daniar  /home/daniar/.ssh
    chown daniar  /home/daniar/.ssh/authorized_keys
    echo "daniar ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers.d/90-cloud-init-users
    exit
    exit


0. Setup zsh [Login on "daniar"]

    ssh 129.114.108.8 

        sudo su
        yum update -y
        yum install zsh -y
        chsh -s /bin/zsh root

        # Break the Copy here ====

        exit
        sudo chsh -s /bin/zsh daniar
        which zsh
        echo $SHELL

        sudo yum install wget git vim zsh -y
 
        # Break the Copy here ====

        printf 'Y' | sh -c "$(wget -O- https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

        /bin/cp ~/.oh-my-zsh/templates/zshrc.zsh-template ~/.zshrc
        sudo sed -i 's|home/daniar:/bin/bash|home/daniar:/bin/zsh|g' /etc/passwd
        sudo sed -i 's|ZSH_THEME="robbyrussell"|ZSH_THEME="risto"|g' ~/.zshrc
        zsh
        exit
        exit

1. Setup passwordless SSH among servers [No-NEED]
    # Go ahead if you guys want to run this as well
    
    # Run at Local
        declare -a arrIP=("192.5.87.198")
        for ip in "${arrIP[@]}"
        do
            echo "$ip"
            scp /Users/daniar/Documents/_CORTX/ssh-key/id_rsa $ip:~/.ssh/
            scp /Users/daniar/Documents/_CORTX/ssh-key/id_rsa.pub $ip:~/.ssh/
            ssh $ip "cd ~/.ssh/; cat id_rsa.pub >> authorized_keys; echo 'StrictHostKeyChecking no' > config; chmod go-rw config"
        done

2. Create loop devices [[AFTER EACH REBOOT]]
    # https://www.thegeekdiary.com
    # If there is only 1 physical storage, you must create loop devices!
        # linux support block device called the loop device, which maps a normal file onto a virtual block device

    # Create a file (25 GB each)

        mkdir -p /mnt/extra/loop-files/
        cd /mnt/extra/loop-files/
        dd if=/dev/zero of=loopbackfile1.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile2.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile3.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile4.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile5.img bs=100M count=250
        
        # check size 
        # du -sh loopbackfile1.img 

            # 1048576000 bytes (1.0 GB) copied, 0.723784 s, 1.4 GB/s
            # 1001M loopbackfile1.img

    # Create the loop device
        cd /mnt/extra/loop-files/
        sudo losetup -fP loopbackfile1.img
        sudo losetup -fP loopbackfile2.img
        sudo losetup -fP loopbackfile3.img
        sudo losetup -fP loopbackfile4.img
        sudo losetup -fP loopbackfile5.img

    # print loop devices 
        losetup -a
            # /dev/loop0: []: (/mnt/extra/loop-files/loopbackfile1.img)
            # /dev/loop1: []: (/mnt/extra/loop-files/loopbackfile2.img)
            # /dev/loop2: []: (/mnt/extra/loop-files/loopbackfile3.img)

    # Format devices into filesystems 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile1.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile2.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile3.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile4.img 
        printf "y" | sudo mkfs.ext4 /mnt/extra/loop-files/loopbackfile5.img 
        lsblk -f

    # mount loop devices

        # mkdir -p /mnt/extra/loop-devs/loop0
        # mkdir -p /mnt/extra/loop-devs/loop1
        # mkdir -p /mnt/extra/loop-devs/loop2
        # mkdir -p /mnt/extra/loop-devs/loop3
        # mkdir -p /mnt/extra/loop-devs/loop4
        # cd /mnt/extra/loop-devs/
        # sudo mount -o loop /dev/loop0 /mnt/extra/loop-devs/loop0
        # sudo mount -o loop /dev/loop1 /mnt/extra/loop-devs/loop1
        # sudo mount -o loop /dev/loop2 /mnt/extra/loop-devs/loop2
        # sudo mount -o loop /dev/loop3 /mnt/extra/loop-devs/loop3
        # sudo mount -o loop /dev/loop4 /mnt/extra/loop-devs/loop4
        lsblk -f
        df -h 

        # remove loop devs [No-NEED]
            # sudo umount /mnt/extra/loop-devs/loop0
            # sudo umount /mnt/extra/loop-devs/loop1
            # sudo umount /mnt/extra/loop-devs/loop2
            # sudo umount /mnt/extra/loop-devs/loop3
            # sudo umount /mnt/extra/loop-devs/loop4
            # sudo losetup -d /dev/loop0
            # sudo losetup -d /dev/loop1
            # sudo losetup -d /dev/loop2
            # sudo losetup -d /dev/loop3
            # sudo losetup -d /dev/loop4
            # rm -rf /mnt/extra/loop-files/*.img

        # check using "lsblk"
            # we will use "loop5" "loop6" "loop7" for Motr 
            # "loop8" for log 


3. Install Kubernetes
    
    sudo mkdir -p /mnt/extra
    sudo chown daniar -R /mnt
    cd /mnt/extra

    # edit the hosts' IP
    sudo bash -c "echo 10.52.0.167 `hostname` >> /etc/hosts"

    # disable firewall
    sudo ufw disable

    # let ipTable see bridged networks
cat <<EOF | sudo tee /etc/modules-load.d/k8s.conf
overlay
br_netfilter
EOF

    # system parameters 
cat <<EOF | sudo tee /etc/sysctl.d/k8s.conf
net.bridge.bridge-nf-call-iptables  = 1
net.bridge.bridge-nf-call-ip6tables = 1
net.ipv4.ip_forward                 = 1
EOF

    # crio
cat <<EOF | sudo tee /etc/modules-load.d/crio.conf
overlay
br_netfilter
EOF

    # kubernetes-cri
cat <<EOF | sudo tee /etc/sysctl.d/99-kubernetes-cri.conf
net.bridge.bridge-nf-call-iptables  = 1
net.ipv4.ip_forward                 = 1
net.bridge.bridge-nf-call-ip6tables = 1
EOF

    # apply configs
    sudo modprobe overlay && sudo modprobe br_netfilter && sudo sysctl --system

    # selinux
    sudo setenforce 0
    sudo sed -i 's/^SELINUX=enforcing$/SELINUX=permissive/' /etc/selinux/config
    swapoff -a
    sed -i '/swap/d' /etc/fstab
    yum repolist -y

    # install kube
cat <<EOF | sudo tee /etc/yum.repos.d/kubernetes.repo
[kubernetes]
name=Kubernetes
baseurl=https://packages.cloud.google.com/yum/repos/kubernetes-el7-\$basearch
enabled=1
gpgcheck=1
repo_gpgcheck=0
gpgkey=https://packages.cloud.google.com/yum/doc/yum-key.gpg https://packages.cloud.google.com/yum/doc/rpm-package-key.gpg
exclude=kubelet kubeadm kubectl
EOF

    # install cri-o
    sudo yum update -y && sudo yum install -y yum-utils nfs-utils tmux
    OS=CentOS_7
    VERSION=1.24
    sudo curl -L -o /etc/yum.repos.d/devel:kubic:libcontainers:stable.repo https://download.opensuse.org/repositories/devel:/kubic:/libcontainers:/stable/$OS/devel:kubic:libcontainers:stable.repo
    sudo curl -L -o /etc/yum.repos.d/devel:kubic:libcontainers:stable:cri-o:$VERSION.repo https://download.opensuse.org/repositories/devel:kubic:libcontainers:stable:cri-o:$VERSION/$OS/devel:kubic:libcontainers:stable:cri-o:$VERSION.repo
    sudo yum install cri-o -y
     

    # install Kubernetes, specify the version as CRI-O
    sudo yum install -y kubelet-1.24.0-0 kubeadm-1.24.0-0 kubectl-1.24.0-0 --disableexcludes=kubernetes

    # download yq 
    cd /mnt/extra
    wget https://github.com/mikefarah/yq/releases/download/v4.25.2/yq_linux_amd64.tar.gz -O - | tar xz 
    sudo mv yq_linux_amd64 /usr/bin/yq

    # edit kubeadm.conf
        # add line before EnvironmentFile: 
        # append to the "last line" ExecStart=/usr/bin/kubelet .... 

        cd /usr/lib/systemd/system/kubelet.service.d/
        sudo sed -i '/EnvironmentFile=-\/var\/lib\/kubelet\/kubeadm-flags.env/i Environment=\"KUBELET_CGROUP_ARGS=--cgroup-driver=systemd\"' 10-kubeadm.conf
        sudo sed -i '$s/$/ $KUBELET_CGROUP_ARGS/' 10-kubeadm.conf

        cat 10-kubeadm.conf| grep "KUBELET_CGROUP_ARGS"

    # enable crio and kubelet 
    sudo systemctl daemon-reload
    sudo systemctl enable crio --now
    sudo systemctl enable kubelet --now

    # init cluster (only on master node)
        sudo su
            kubeadm init --pod-network-cidr=192.168.0.0/16
            mkdir -p $HOME/.kube && sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
            sudo chown $(id -u):$(id -g) $HOME/.kube/config
            
            echo -e "export KUBECONFIG=/etc/kubernetes/admin.conf \nalias kc=kubectl \nalias all=\"kubectl get pods -A -o wide\"" >> /etc/bashrc && source /etc/bashrc
            echo -e "export KUBECONFIG=/etc/kubernetes/admin.conf \nalias kc=kubectl \nalias all=\"kubectl get pods -A -o wide\"" >> /etc/zshrc && source /etc/zshrc

            # for multiple node is mandatory
            kubectl create -f https://projectcalico.docs.tigera.io/manifests/tigera-operator.yaml
            cd /mnt/extra
            wget https://gist.githubusercontent.com/faradawn/2288618db8ad0059968f48b6647732f9/raw/133f7f5113b4bc76f06dd5240ae7775c2fb74307/custom-resource.yaml
            kubectl create -f custom-resource.yaml
        exit 

        # to enable checking kubectl on non root
        mkdir -p $HOME/.kube && sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
        sudo chown $(id -u):$(id -g) $HOME/.kube/config
        sudo chown $USER /etc/kubernetes/admin.conf

        # turn off / shutdown / kill kubeadm
            # sudo kubeadm reset

    # check whether it is installed
    sudo kubectl get nodes
        # NAME                 STATUS   ROLES           AGE   VERSION
        # cortx-k8.novalocal   Ready    control-plane   5s    v1.24.0

4. Deploy CORTX to kubernetes
    
    # clone k8s repo and download solution.example.yaml
    cd /mnt/extra
    git clone -b main https://github.com/Seagate/cortx-k8s

    # modify "solution.yaml" [MANUALLY]
        cd /mnt/extra/cortx-k8s/k8_cortx_cloud
        sudo sed -i 's|csm_mgmt_admin_secret: null|csm_mgmt_admin_secret: Cortx123\!|g' solution.example.yaml
        
        vim solution.example.yaml

            # 1. Search for "?nodes"
                # Change the nodes -> make sure it points to the desired nodes 
                # for 1 node deployment, the node1: name should be `hostname`
                # then delete other nodes 

            # 2. Only use 1 CVG! (otherwise it failed on "deploy-cortx-cloud")
                # delete other CVG
                # storage:
                    # cvg1 

            # 3. Search for "?sd"
                # we will use "dev/loop0" "dev/loop1" "dev/loop2" "dev/loop3" for Motr 
                # make sure that those devices existed, otherwise follow "Create loop devices"
                # Changes the sdc to loop4 and so on!
                    # svg1 uses "dev/loop4" "dev/loop5"
                    # size is 25Gi 
                    # remove other sd* 

        mv solution.example.yaml solution.yaml
        cat solution.yaml | grep "loop"

    # Run prerequisite [After each shutdown]
    cd /mnt/extra/cortx-k8s/k8_cortx_cloud
    sudo ./prereq-deploy-cortx-cloud.sh -d /dev/loop4 -s solution.yaml

        # check the provisioner is mounted or not
        lsblk -f | grep "/mnt/fs-local-volume"
            # loop8  ext4        6c0abb28-82da-4f9c-96d1-4ae070d8b57c /mnt/fs-local-volume

    sudo su
        # untaint master [run-once]
            kubectl taint node `hostname -s` node-role.kubernetes.io/master:NoSchedule-
            kubectl taint node `hostname -s` node-role.kubernetes.io/control-plane:NoSchedule-

        # restart core-dns pods [run-once]
        kubectl rollout restart -n kube-system deployment/coredns

        # START k8 deployment [10 mins]     # Need a steady ssh connection!!
        time ./deploy-cortx-cloud.sh solution.yaml          # Run this within VSCODE terminal!!!
    exit

    # Shutdown/reset 
        # cd /mnt/extra/cortx-k8s/k8_cortx_cloud
        # sudo ./destroy-cortx-cloud.sh
        # sudo umount /mnt/fs-local-volume

5. Establish CORTX-K8 Credential [in-SUDO]
    # Do this after each k8 restart 
    
    sudo su 

    # login to CSM to get the Bearer token 
    export CSM_IP=`kubectl get svc cortx-control-loadbal-svc -ojsonpath='{.spec.clusterIP}'`
    echo $CSM_IP

    tok=$(curl -d '{"username": "cortxadmin", "password": "Cortx123!"}' https://$CSM_IP:8081/api/v2/login -k -i | grep -Po '(?<=Authorization: )\w* \w*') && echo $tok
        # Bearer 936b8c4d8756421682fdb059eec75c4b
    # create and check IAM user
    curl -X POST -H "Authorization: $tok" -d '{ "uid": "12345678", "display_name": "gts3account", "access_key": "gregoryaccesskey", "secret_key": "gregorysecretkey" }' https://$CSM_IP:8081/api/v2/iam/users -k

    curl -H "Authorization: $tok" https://$CSM_IP:8081/api/v2/iam/users/12345678 -k -i | grep "user_id"
        # {"tenant": "", "user_id": "12345678", ....

6. Install aws cli [No-NEED]

    # comparable to MinIO interface
    pip3 install awscli awscli-plugin-endpoint
    aws configure set plugins.endpoint awscli_plugin_endpoint
    aws configure set default.region us-east-1
    aws configure set aws_access_key_id gregoryaccesskey
    aws configure set aws_secret_access_key gregorysecretkey

    # find PORT and IP
    kubectl describe svc cortx-io-svc-0 | grep -Po 'NodePort.*rgw-http *[0-9]*'
    export PORT=30773 (NodePort - cortx-rgw-http - 30056/TCP (PORT=30056))
    export IP=192.168.84.128 (ifconfig, tunl0, IPIP tunnel)

    # test upload bucket
    aws s3 mb s3://test-bucket --endpoint-url http://$IP:$PORT
    echo "hello world" >> foo.txt
    aws s3 cp foo.txt s3://test-bucket --endpoint-url http://$IP:$PORT
    aws s3 ls s3://test-bucket --endpoint-url http://$IP:$PORT
    aws s3 rm s3://test-bucket/foo.txt --endpoint-url http://$IP:$PORT
    aws s3 rb s3://test-bucket --endpoint-url http://$IP:$PORT
    aws s3 ls --endpoint-url http://$IP:$PORT

7. Run Clear Cache loop  
    # Run this in the VSCode terminal, otherwise the process will terminate easily
    # Background Task [On other terminal]
    # keep this script running while benchmarking 

    cd /mnt/extra/cortx-motr/motr/examples/script/
    sudo ./free_page_cache.sh 0.25

8. S3 Benchmark CORTX-K8  [in-SUDO]
    
    sudo su

    # Install s3bench
        cd /mnt/extra
        yum install -y go
        wget https://github.com/Seagate/s3bench/releases/download/v2022-03-14/s3bench.2022-03-14
        chmod +x s3bench.2022-03-14 
        mv s3bench.2022-03-14 s3bench

    # Run benchmark

        IP=`ifconfig | grep "inet 192" | awk '{print $2}'`
        PORT=`kubectl describe svc | grep "rgw-http  " | grep "NodePort" | grep -o -E '[0-9]+'`
        echo $IP:$PORT 

        cd /mnt/extra
        ./s3bench -accessKey gregoryaccesskey -accessSecret gregorysecretkey -bucket loadgen -endpoint http://$IP:$PORT -numClients 1 -numSamples 500 -objectNamePrefix=loadgen -objectSize 1Kb -region us-east-1 -o test1.log

        cd /mnt/extra
        mkdir -p /mnt/extra/logs/
        # "4Kb" "8Kb" "16Kb" "32Kb" "64Kb" "128Kb" "256Kb" "512Kb" "1Mb" "2Mb" "4Mb" "8Mb" "16Mb"
        blockSizeArr=("16Mb")
        for size in "${blockSizeArr[@]}"
        do
            echo "$size"
            ./s3bench -accessKey gregoryaccesskey -accessSecret gregorysecretkey -bucket loadgen -endpoint http://$IP:$PORT -numClients 1 -numSamples 1000 -objectNamePrefix=loadgen -objectSize $size -region us-east-1 -o logs/s3bench-$size.log
        done

    # Error Debug
        # 1. panic: Failed to create bucket: InvalidAccessKeyId
            # Solution: Follow the "Establish CORTX-K8 Credential"

9. Download all the logs 
    # Download logs / scp logs download the logs to local

    log_dir="cortx-k8-s3bench-v0"
    mkdir -p ~/Documents/_CORTX/logs/$log_dir
    rsync -Pav daniar@192.5.86.172:/mnt/extra/logs/ ~/Documents/_CORTX/logs/$log_dir
    # scp -rp daniar@192.5.86.172:/mnt/extra/logs/ ~/Documents/_CORTX/logs/$log_dir





\"
"
========================================================================================
                        Run a Single Node Ceph (Loops)
========================================================================================

# project ..849 @UC "Skylake"

# Create Reservation
    # https://chi.tacc.chameleoncloud.org/project/
    
    1. Reserve Physical Host
        # click "Leases" => "+ Create Lease"
        # lease name = "dan-cortx"
            "$node_type", "compute_skylake"
            Max lease: 
                7 days
    2. Reserve Network [No-NEED]

# Launching an Instance -> name it as "node-1" "node-2" etc
    # In the sidebar, click Compute, then click Instances
    # Click on the Launch Instance
        # pick the correct reservation 
        # count = 1 (for singlenode)
        # Image: "CC-CentOS7"    # need 1062?? no
    # Select "sharednet1"
    # Choose the ssh key
        # "dan-macpro"

# Allocate floating IPs
    # Book the IP interface
        # Click "Network -> Floating IPs -> Allocate IP To Project"
        # Write description
        # Click "Allocate IP"

    # click "Associate" OR click "attach interface"
        # Click "Network -> Floating IPs"

    # ony 6 available public interfaces
    # wait a few minutes
    # is it Spawning?? https://chi.tacc.chameleoncloud.org/project/instances/

# If you can't access the cc user on SSH
    # open the console terminal via the website
    # edit the .ssh/authorized_keys and add your pub_key manually!!!
    # now, the hostname will be different 
        # e.g. from dan-storage to zhenz-test
    # run "sudo dhclient" from the web-based ssh console

00. Preparation [Login as "cc"]
    # Use cc user!!
    ssh cc@129.114.108.8
    # Setup disk 
        # check if there is already mounted disk
        df -H
            # /dev/sda1       251G  2.8G  248G   2% /
            # should be enough

    # Setup user daniar 
    sudo adduser daniar
    sudo usermod -aG wheel daniar
    sudo su 
    cp -r /home/cc/.ssh /home/daniar
    chmod 700  /home/daniar/.ssh
    chmod 644  /home/daniar/.ssh/authorized_keys
    chown daniar  /home/daniar/.ssh
    chown daniar  /home/daniar/.ssh/authorized_keys
    echo "daniar ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers.d/90-cloud-init-users
    exit
    exit


0. Setup zsh [Login on "daniar"]

    ssh 129.114.108.8 

        sudo su
        yum update -y
        yum install zsh -y
        chsh -s /bin/zsh root

        # Break the Copy here ====

        exit
        sudo chsh -s /bin/zsh daniar
        which zsh
        echo $SHELL

        sudo yum install wget git vim zsh -y
 
        # Break the Copy here ====

        printf 'Y' | sh -c "$(wget -O- https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

        /bin/cp ~/.oh-my-zsh/templates/zshrc.zsh-template ~/.zshrc
        sudo sed -i 's|home/daniar:/bin/bash|home/daniar:/bin/zsh|g' /etc/passwd
        sudo sed -i 's|ZSH_THEME="robbyrussell"|ZSH_THEME="risto"|g' ~/.zshrc
        zsh
        exit
        exit

1. Setup passwordless SSH among servers [No-NEED]
    # Go ahead if you guys want to run this as well
    
    # Run at Local
        declare -a arrIP=("192.5.86.186" "192.5.86.195" "192.5.86.194")
        for ip in "${arrIP[@]}"
        do
            echo "$ip"
            scp /Users/daniar/Documents/_CORTX/ssh-key/id_rsa $ip:~/.ssh/
            scp /Users/daniar/Documents/_CORTX/ssh-key/id_rsa.pub $ip:~/.ssh/
            ssh $ip "cd ~/.ssh/; cat id_rsa.pub >> authorized_keys; echo 'StrictHostKeyChecking no' > config; chmod go-rw config"
        done


2. Create XFS loop devices [[AFTER EACH REBOOT]]
    # https://www.thegeekdiary.com/how-to-create-virtual-block-device-loop-device-filesystem-in-linux/
    # If there is only 1 physical storage, you must create loop devices!
        # linux support block device called the loop device, which maps a normal file onto a virtual block device

    # Create a file (25 GB each)

        sudo chown daniar -R /mnt/
        mkdir -p /mnt/extra/loop-files/
        cd /mnt/extra/loop-files/
        dd if=/dev/zero of=loopbackfile1.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile2.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile3.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile4.img bs=100M count=250
        dd if=/dev/zero of=loopbackfile5.img bs=100M count=250
        
        # check size 
        # du -sh loopbackfile1.img 

            # 1048576000 bytes (1.0 GB) copied, 0.723784 s, 1.4 GB/s
            # 1001M loopbackfile1.img

    # Create the loop device
        cd /mnt/extra/loop-files/
        sudo losetup -fP loopbackfile1.img
        sudo losetup -fP loopbackfile2.img
        sudo losetup -fP loopbackfile3.img
        sudo losetup -fP loopbackfile4.img
        sudo losetup -fP loopbackfile5.img

    # print loop devices 
        losetup -a
            # /dev/loop0: []: (/mnt/extra/loop-files/loopbackfile1.img)
            # /dev/loop1: []: (/mnt/extra/loop-files/loopbackfile2.img)
            # /dev/loop2: []: (/mnt/extra/loop-files/loopbackfile3.img)

    # Format devices into filesystems 
        printf "y" | sudo mkfs -t xfs -q /mnt/extra/loop-files/loopbackfile1.img 
        printf "y" | sudo mkfs -t xfs -q /mnt/extra/loop-files/loopbackfile2.img 
        printf "y" | sudo mkfs -t xfs -q /mnt/extra/loop-files/loopbackfile3.img 
        printf "y" | sudo mkfs -t xfs -q /mnt/extra/loop-files/loopbackfile4.img 
        printf "y" | sudo mkfs -t xfs -q /mnt/extra/loop-files/loopbackfile5.img 
        lsblk -f

    # mount loop devices [No-NEED]

        # mkdir -p /mnt/extra/loop-devs/loop0
        # mkdir -p /mnt/extra/loop-devs/loop1
        # mkdir -p /mnt/extra/loop-devs/loop2
        # mkdir -p /mnt/extra/loop-devs/loop3
        # mkdir -p /mnt/extra/loop-devs/loop4
        # cd /mnt/extra/loop-devs/
        # sudo mount -o loop /dev/loop0 /mnt/extra/loop-devs/loop0
        # sudo mount -o loop /dev/loop1 /mnt/extra/loop-devs/loop1
        # sudo mount -o loop /dev/loop2 /mnt/extra/loop-devs/loop2
        # sudo mount -o loop /dev/loop3 /mnt/extra/loop-devs/loop3
        # sudo mount -o loop /dev/loop4 /mnt/extra/loop-devs/loop4
        lsblk -f
        df -h 

        # remove loop devs [No-NEED]
            # sudo umount /mnt/extra/loop-devs/loop0
            # sudo umount /mnt/extra/loop-devs/loop1
            # sudo umount /mnt/extra/loop-devs/loop2
            # sudo umount /mnt/extra/loop-devs/loop3
            # sudo umount /mnt/extra/loop-devs/loop4
            # sudo losetup -d /dev/loop0
            # sudo losetup -d /dev/loop1
            # sudo losetup -d /dev/loop2
            # sudo losetup -d /dev/loop3
            # sudo losetup -d /dev/loop4
            # rm -rf /mnt/extra/loop-files/*.img

        # check using "lsblk"
            # we will use "loop5" "loop6" "loop7" for Motr 
            # "loop8" for log 

2. Install Ceph 
    # https://docs.ceph.com/en/mimic/start/quick-start-preflight/

    # dependencies
        # install subscription-manager
        sudo yum update
        sudo yum makecache
        sudo yum -y install subscription-manager
        sudo yum -y install firewalld

    # Preps
    sudo subscription-manager repos --enable=rhel-7-server-extras-rpms
    sudo yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm

    # Add ceph repo 
cat <<EOF | sudo tee /etc/yum.repos.d/ceph.repo
[ceph-noarch]
name=Ceph noarch packages
baseurl=https://download.ceph.com/rpm-luminous/el7/noarch
enabled=1
gpgcheck=1
type=rpm-md
gpgkey=https://download.ceph.com/keys/release.asc
EOF

    # Update your repository and install ceph-deploy:
        sudo yum -y install ceph-deploy

3. Setup config 
    # https://docs.ceph.com

    # install ntp 
    sudo yum -y install ntp ntpdate ntp-doc
    sudo ntpdate 0.us.pool.ntp.org
    sudo hwclock --systohc
    sudo systemctl enable ntpd.service
    sudo systemctl start ntpd.service


    # Install ssh server 
    sudo yum -y install openssh-server

    # set etc/hosts
    sudo bash -c "echo 10.140.82.40 node-1 >> /etc/hosts"
    # sudo bash -c "echo 10.140.82.255 node-2 >> /etc/hosts"
    # sudo bash -c "echo 10.140.83.222 node-3 >> /etc/hosts"

    # update ssh config
cat <<EOF | sudo tee ~/.ssh/config
Host node-1
   Hostname node-1
   User daniar
Host node-2
   Hostname node-2
   User daniar
Host node-3
   Hostname node-3
   User daniar
EOF

    # disable firewall 
    sudo ufw disable
    sudo firewall-cmd --state

    # selinux
    sudo setenforce 0
    sudo sed -i 's/^SELINUX=enforcing$/SELINUX=disabled/' /etc/selinux/config

    # enable plugin-priorities
    sudo yum -y install yum-plugin-priorities 
        # --enablerepo=rhel-7-server-optional-rpms


4. Ceph Deploy [On-MASTER]
    # https://ralph.blog.imixs.com/2020/02/28/howto-install-ceph-on-centos-7/
    # https://www.linuxtechi.com/install-configure-ceph-cluster-centos-7/

    sudo mkdir -p /mnt/extra/ceph_cluster
    cd /mnt/extra/ceph_cluster
    # sudo ceph-deploy new node-1                        # be ready to type "yes"
    sudo ceph-deploy new node-1 node-2 node-3        # be ready to type "yes"
        # ... Writing initial config to ceph.conf...

    # verify the Ceph configuration
        cat ceph.conf | grep mon_host

    # Patch [Run in EACH NODE]
        # workaround: https://tracker.ceph.com/issues/12694
        sudo mv /etc/yum.repos.d/ceph.repo /etc/yum.repos.d/ceph-deploy.repo

    # Install Ceph [in-SUDO]

        cd /mnt/extra/ceph_cluster
        # ceph-deploy install --release luminous node-1 
        ceph-deploy install --release luminous node-1 node-2 node-3 
            # ceph version 12.2.13 (584a20eb0237c657dc0567da126be145106aa47e) luminous (stable)

        # Patch [Run in EACH NODE]
            # https://www.netways.de/en/blog/2018/11/14/ceph-mimic-using-loop-devices-as-osd/
            sudo sed -i "s|return TYPE == 'disk'|return TYPE == 'disk' or TYPE == 'loop' or TYPE == 'part'|g" /usr/lib/python2.7/site-packages/ceph_volume/util/disk.py
            cat /usr/lib/python2.7/site-packages/ceph_volume/util/disk.py | grep "TYPE == 'loop'"

        # Deploy the initial monitor(s) and gather the keys:
        ceph-deploy mon create-initial
            # Monitor and OSD better not in the same node [https://documentation.suse.comstorage-bp-hwreq.html]
                # Strongly recommended to have a cluster (>1 node) deployment!!
            # failed if only deployed in single node: 
                # [node-1][ERROR ] admin_socket: exception getting command descriptions: [Errno 2] No such file or directory
                # [ceph_deploy.mon][ERROR ] Some monitors have still not reached quorum:
                # [ceph_deploy.mon][ERROR ] node-1

        # copy the configuration file to all ceph nodes
        ceph-deploy admin node-1 node-2 node-3

        # Deploy a manager daemon
        ceph-deploy mgr create node-1

        # Next create metadata servers:
        ceph-deploy mds create node-1 node-2 node-3

        # check status 
        sudo ceph status 

              # services:
                # mon: 3 daemons, quorum node-1,node-2,node-3
                # mgr: no daemons active
                # osd: 0 osds: 0 up, 0 in

              # data:
                # pools:   0 pools, 0 pgs
                # objects: 0 objects, 0B
                # usage:   0B used, 0B / 0B avail
                # pgs:

5. Create Object Store Daemons (OSDs)
    # will use /dev/loop5, /dev/loop6, /dev/loop7, /dev/loop8, /dev/loop9
    
    cd /mnt/extra/ceph_cluster
    # ceph-deploy osd create --data /dev/loop1 node-3 --filestore --journal /dev/loop3 --fs-type xfs
    ceph-deploy osd create --data /dev/loop1 node-1
    ceph-deploy osd create --data /dev/loop2 node-1
    ceph-deploy osd create --data /dev/loop3 node-1
    ceph-deploy osd create --data /dev/loop1 node-2
    ceph-deploy osd create --data /dev/loop2 node-2
    ceph-deploy osd create --data /dev/loop3 node-2
    ceph-deploy osd create --data /dev/loop1 node-3
    ceph-deploy osd create --data /dev/loop2 node-3
    ceph-deploy osd create --data /dev/loop3 node-3

    # create pool 
        # https://docs.ceph.com/en/latest/rados/operations/pools/
        # https://computingforgeeks.com/create-a-pool-in-ceph-storage-cluster/
            # replicated, not erasure
        sudo ceph osd pool create mypool 100 100 replicated

        # Associate Pool to Application
        sudo ceph osd pool application enable mypool rgw

    # verify cluster status 
    sudo ceph -s
        # data:
            # pools:   1 pools, 100 pgs
            # objects: 0 objects, 0B
            # usage:   9.08GiB used, 211GiB / 220GiB avail
            # pgs:     100 active+clean

    sudo ceph health
        # HEALTH_WARN clock skew detected on mon.node-2, mon.node-3
            # sudo timedatectl set-ntp false
            # sudo ntpdate -u pool.ntp.org 
            # sudo timedatectl set-ntp true
    
    sudo ceph health        # check after 30s, the cluster need time to check the clock skew

6. Ceph Dashboard 
    # https://ralph.blog.imixs.com/2020/02/28/howto-install-ceph-on-centos-7/
    # Enable Dashboard: https://docs.ceph.com/en/quincy/mgr/dashboard/
    # Works: https://stackoverflow.com/questions/55549420/

    cd /mnt/extra/ceph_cluster
    sudo ceph mgr module enable dashboard
        # ceph mgr module disable dashboard

    sudo ceph mgr services          # wait 1 min 
        # {
        #    "dashboard": "http://node-1.novalocal:7000/"
        # }

    # Port Forwarding [Run at LOCAL]

        ssh -L 9999:localhost:7000  daniar@192.5.86.186

        # visit this at local: http://localhost:9999/

            # Copyright © 2016 by Ceph Contributors. Free software (LGPL 2.1)
            # ceph version 12.2.13 (584a20eb0237c657dc0567da126be145106aa47e) luminous (stable)


7. Start Rados Gateway (rgw)
    # Just a sneak peek: https://www.youtube.com/watch?v=6uX23Q3y_SU
    # https://access.redhat.com/documentation/en-us/red_hat_ceph_storage/3/html/installation_guide_for_red_hat_enterprise_linux/manually-installing-ceph-object-gateway
    # https://docs.ceph.com/en/mimic/start/quick-ceph-deploy/

    # install rgw daemon 
        sudo yum -y install ceph-radosgw

    # install rgw sync agent 
        sudo yum -y install radosgw-agent

    # Create a keyring
        sudo ceph-authtool --create-keyring /etc/ceph/ceph.client.radosgw.keyring
        sudo chmod +r /etc/ceph/ceph.client.radosgw.keyring

    # Generate a Ceph Object Gateway user name
        sudo ceph-authtool /etc/ceph/ceph.client.radosgw.keyring -n client.radosgw.node-1 --gen-key

    # Add capabilities to the key:
        sudo ceph-authtool -n client.radosgw.node-1 --cap osd 'allow rwx' --cap mon 'allow rwx' /etc/ceph/ceph.client.radosgw.keyring

    # add the key to Ceph Cluster
        sudo ceph -k /etc/ceph/ceph.client.admin.keyring auth add client.radosgw.node-1 -i /etc/ceph/ceph.client.radosgw.keyring

    # Distribute the keyring to the gateway host 
        # no need because the admin node is gateway node 

    # Add a Gateway Configuration to Ceph
        # The host variables determine which host runs each instance of a radosgw daemon
cat <<EOF | sudo tee -a /etc/ceph/ceph.conf
[client.radosgw.node-1]
host = node-1
keyring = /etc/ceph/ceph.client.radosgw.keyring
rgw socket path = ""
log file = /var/log/radosgw/client.radosgw.node-1.log
rgw frontends = "civetweb port=8080"
rgw print continue = false
EOF

    # Distribute updated Ceph configuration file
        ceph-deploy --overwrite-conf config pull node-1 
            # Got /etc/ceph/ceph.conf from node-1
        ceph-deploy --overwrite-conf config push node-2 node-3

    # Create Data Directory
        sudo mkdir -p /var/lib/ceph/radosgw/ceph-radosgw.node-1
        sudo mkdir -p /var/log/radosgw/

    # Adjust Directory Permissions
        sudo chown daniar /var/run/ceph
        sudo chown daniar /var/log/radosgw/

    # Start radosgw service
        # https://tracker.ceph.com/issues/24265 => becareful with the @xxxx

        sudo systemctl start ceph-radosgw@radosgw.node-1
        # sudo systemctl restart ceph-radosgw@radosgw.node-1
        sudo systemctl status ceph-radosgw@radosgw.node-1
        # sudo systemctl stop ceph-radosgw@radosgw.node-1
        
        sudo firewall-cmd --state
            # not running

    # check status 
        curl node-1:8080

            # <?xml version="1.0" encoding="UTF-8"?><ListAllMyBucketsResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/"><Owner><ID>anonymous</ID><DisplayName></DisplayName></Owner><Buckets></Buckets></ListAllMyBucketsResult>%

    # Port Forwarding [Run at LOCAL]

        ssh -L 8888:localhost:8080  daniar@192.5.86.186

        # visit this at local: http://localhost:8888/

            # <ListAllMyBucketsResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
            #    <Owner>
            #        <ID>anonymous</ID>
            #        <DisplayName/>
            #    </Owner>
            # <Buckets/>
            # </ListAllMyBucketsResult>


8. Prepare Credentials (S3 interface)
    # https://access.redhat.com/documentation/en-us/red_hat_ceph_storage/1.2.3/html-single/ceph_object_gateway_for_centos_x86_64/index
    
    # Create a radosgw user for S3 access
        sudo radosgw-admin user create --uid="testuser" --display-name="First User"
        sudo radosgw-admin user list
        sudo radosgw-admin user info --uid=testuser
            # sudo radosgw-admin user rm --uid=testuser

    # Create a Swift user
        sudo radosgw-admin subuser create --uid=testuser --subuser=testuser:swift --access=full
        
        # get the "access_key" and "secret_key"
            # ...
            # "keys": [
            #    {
                    "user": "testuser",
                    "access_key": "Y833676BM11O30BTS5FC",
                    "secret_key": "iBAx83AbY0hUUm9BCRqmwTKbdbod3cbKWU7PNR7U"
            #    }
            #],

    # Create the secret key:
        sudo radosgw-admin key create --subuser=testuser:swift --key-type=swift --gen-secret

            # "swift_keys": [
            #    {
                    "user": "testuser:swift",
                    "secret_key": "TD2f8KR1zlQJ3elNWivIAi8tZgI8Cnlh7Yx61HB5"
            #    }
            #],

    # Test S3 Access 

        # dependencies
        sudo yum -y install python-boto

        # create test client
        cd /mnt/extra/ceph_cluster
cat <<EOF | sudo tee s3test.py
import boto
import boto.s3.connection
access_key = 'Y833676BM11O30BTS5FC'
secret_key = 'iBAx83AbY0hUUm9BCRqmwTKbdbod3cbKWU7PNR7U'
conn = boto.connect_s3(
    aws_access_key_id = access_key,
    aws_secret_access_key = secret_key,
    host = "10.140.82.40",
    port = 8080,
    is_secure=False,
    calling_format = boto.s3.connection.OrdinaryCallingFormat(),
)
bucket = conn.create_bucket('my-new-bucket')
for bucket in conn.get_all_buckets():
    print "{name}\t{created}".format(
        name = bucket.name,
        created = bucket.creation_date,
)
EOF

        # run s3 client 
            cd /mnt/extra/ceph_cluster
            python2 s3test.py
                # my-new-bucket 2022-07-22T16:34:07.905Z

    # Test swift access
        # dependencies
            sudo yum -y install python-setuptools
            # sudo pip install ez_setup
            pip install setuptools
            pip install python-swiftclient

        # run test 
            swift -A http://10.140.82.40:8080/auth/1.0 -U testuser:swift -K 'TD2f8KR1zlQJ3elNWivIAi8tZgI8Cnlh7Yx61HB5' list
                # my-new-bucket


9. Clone CORTX-bench-extra
    # Change the "daniarherikurniawan" with your github username
    # Set passwordless github push/pull 
    cd /mnt/extra
    git config --global user.email "ddhhkk2@gmail.com"
    git config --global user.name "daniarherikurniawan"
    git config --global credential.helper store         # store pass in ~/.git-credentials as plain text format.

    cd /mnt/extra 
    git clone https://daniarherikurniawan@github.com/daniarherikurniawan/cortx-bench-extra.git
    
    # Run clear cache loop  
    cd /mnt/extra/cortx-bench-extra/script  
    sudo ./free_page_cache.sh 0.25

        
10. Benchmark Ceph using S3bench
    
    # Install s3bench
        # https://github.com/markhpc/hsbench

        sudo yum -y install jq
        sudo yum install -y go
            # sudo yum remove go

        go env 
            # GOROOT="/usr/lib/golang"
            # GOPATH="/root/go"

        which go
        sudo go install github.com/aws/aws-sdk-go@latest
        sudo find / -type d -name 'aws-sdk-go'
            # /root/go/pkg/mod/cache/download/github.com/aws/aws-sdk-go

        # Install hsbench
            # https://medium.com/@sherlock297/go-get-installing-executables-with-go-get-in-module-mode-is-deprecated-de3a30439596
        sudo go install github.com/markhpc/hsbench@latest 
        sudo find / -name 'hsbench'
            # /root/go/pkg/mod/cache/download/github.com/markhpc/hsbench
        
        sudo ls /root/go/bin
            # hsbench
        
    # Run benchmark

        # -z : Size of objects in bytes with postfix K, M, and G (default "1M")
        # -n : Maximum number of objects <-1 for unlimited> (default -1)
        # -t : Number of threads to run (default 1)
        # -b : Number of buckets to distribute IOs across (default 1)
        # -ri : Number of seconds between report intervals (default 1)
        # -d : Maximum test duration in seconds <-1 for unlimited> (default 60)

        sudo su

        IP=`ifconfig | grep "inet 10" | awk '{print $2}'`
        PORT=8080
        echo $IP:$PORT 
        daniarsecretkey="TD2f8KR1zlQJ3elNWivIAi8tZgI8Cnlh7Yx61HB5"
        access_key="Y833676BM11O30BTS5FC"
        secret_key="iBAx83AbY0hUUm9BCRqmwTKbdbod3cbKWU7PNR7U"

        cd /root/go/bin
        ./hsbench -a $access_key -s $secret_key -u http://$IP:$PORT  -z 4K -t 1 -n 10000 -b 100 -d -1 -m cxipgdcx -ri 10 -j test.log 
            # Ops: 100 -> means there are 100 requests

        # show result 
        cat test.log| jq | grep TOTAL -A 8

            # --
            #    "IntervalName": "TOTAL",
            #    "Seconds": 40.772364648,
            #    "Mode": "GET",
            #    "Ops": 1000,
            #    "Mbps": 0.0958063147360675,
            #    "Iops": 24.52641657243328,
            #    "MinLat": 0.673861,
            #    "AvgLat": 1.200316826,
            #    "NinetyNineLat": 2.327649,
            # --


        cd /root/go/bin
        mkdir -p /root/go/bin/logs/
        # "4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" "1M" "2M" "4M" "8M" "16M"
        blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
        for size in "${blockSizeArr[@]}"
        do
            echo "$size"
            ./hsbench -a $access_key -s $secret_key -u http://$IP:$PORT  -z $size -t 1 -n 10000 -b 100 -d -1 -m cxipgdcx -ri 10 -j logs/hsbench-raw-$size.log
            cat logs/hsbench-raw-$size.log | jq | grep TOTAL -A 8 > logs/hsbench-$size.log
        done

    
        blockSizeArr=( "1M" "2M" "4M" "8M" "16M")
        for size in "${blockSizeArr[@]}"
        do
            echo "$size"
            ./hsbench -a $access_key -s $secret_key -u http://$IP:$PORT  -z $size -t 1 -n 2000 -b 100 -d -1 -m cxipgdcx -ri 10 -j logs/hsbench-raw-$size.log
            cat logs/hsbench-raw-$size.log | jq | grep TOTAL -A 8 > logs/hsbench-$size.log
        done

11. Setup Librados Client [In SUDO]
    # Python version
	sudo yum install python-rados
    sudo yum install python3-rados

    # Using client.admin user
    # Assuming the client.admin keyring is inside /mnt/extra/ceph_cluster
    # Assuming mon_host is already set inside ceph.conf
    # Check whether both keyring and mon_host is set inside ceph configuration file
	    cat ceph.conf
        # Look for:
            # mon_host = ...
            # keyring = ...
        # If not there,
            # If the keyring isn't at /mnt/extra/ceph_cluster
            # Getting the location of keyring, should be inside ceph_cluster
                cd /mnt/extra/ceph_cluster
                ls
            # If mon_host hasn't been set yet
            # Getting the monitor's ip
                ceph mon stat

            nano ceph.conf
            # copy:
                # mon_host = 10.52.3.162,10.52.3.73
                # keyring = /mnt/extra/ceph_cluster/ceph.client.admin.keyring
    
    # Create new python script
        nano ceph-client.py
        # paste code

    # Alternatively, run the python script pulled from cortx-bench-extra repo
        cd /mnt/extra/cortx-bench-extra/librados-clients/

    # Run the script
        python2 ceph-client.py

12. Benchmark Ceph (Librados)
    # Benchmark with RADOS BENCH
        # Ref: https://access.redhat.com/documentation/en-us/red_hat_ceph_storage/1.3/html/administration_guide/benchmarking_performance, 
        #      https://www.sebastien-han.fr/blog/2012/08/26/ceph-benchmarks/
        # If error : HEALTH_WARN Degraded data redundancy: ..., run
            # ceph tell mon.\* injectargs '--mon-allow-pool-delete=true'
            # ceph osd pool delete testbench testbench --yes-i-really-really-mean-it

        # Create new storage pool
            ceph osd pool create testbench 100 100

        # Drop FS Cache
            sudo echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync

        # Benchmarking Write (100s)
            # -p : pool name
            # 100 : time in seconds
            # -t : number of threads
            # -b : block size (only for write)

            blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
            for size in "${blockSizeArr[@]}"
            do
                echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
                echo "$size"
                rados bench -p testbench 100 write -t 1 -b $size
            done

        # Generate data
            rados bench -p testbench 10 write --no-cleanup

        # Benchmarking Read
            rados bench -p testbench 50 rand -t 1

    # Benchmark with Bonnie++
        # Ref: https://www.jamescoyle.net/how-to/599-benchmark-disk-io-with-dd-and-bonnie
        #      https://www.sebastien-han.fr/blog/2012/08/26/ceph-benchmarks/

        # Install bonnie++
        sudo yum -y install bonnie++

        sudo su
        cd /mnt/extra/ceph_cluster
        free -m
        #                total        used        free      shared  buff/cache   available
        # Mem:         191796        3125      115166          34       73503      187901
        # Here we have 191796 MB
        # For minimizing inaccuracy due to OS caching, filesize (-s) should be minimum 2x of total mem
        # So filesize has to be minimum 383592 MB
        # A problem since:
        # 1. Has to be power of 2
        # 2. We have to do 4MB, 8MB, dst
            # Limit the size
                # Ex: bonnie++ -d /opt/ -r A -s B -n 1 -f -b -u root
                # Limit RAM to A, bench size B

        sudo echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
        bonnie++ -d /mnt/extra/ -r 2K -s 4K -n 1 -f -b -u root -m Bonnie4 -x 5000

        blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
        val=1
        for size in "${blockSizeArr[@]}"
        do
            echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
            val=$(($val*2))
            valtemp="$val""K"
            echo "RAM $valtemp & BENCHFILE $size"
            bonnie++ -d /mnt/extra/ -r $valtemp -s $size -n 1 -f -b -u root -m Bonnie$size
        done
    
    # Benchmark using Python Code
        cd /mnt/extra/cortx-bench-extra/librados-clients/
        python3 ceph-client-bench.py

    # C++ version
        sudo yum install librados2-devel

    # Use the same pre-check as python VERSION
    # Compile the c++ file
        g++ -std=c++11 -g -c ceph-client.cpp -o ceph-client.o
        g++ -std=c++11 -g ceph-client.o -lrados -o ceph-client

    # Run the binary file
        sudo ./ceph-client >> test.log

13. Setup RADOS Block Device (RBD)
    # Using client.admin user
    # Ref: https://www.bookstack.cn/read/ceph-en/855793c9b7b2e403.md
    sudo su
    cd /mnt/extra/ceph_cluster

    # To free up OSDs, delete previous pool
    ceph osd pool delete rbdpool rbdpool --yes-i-really-really-mean-it

    # The pool 'mypool' already registered with rgw, so make another pool
    sudo ceph osd pool create rbdpool 100 100 replicated
    sudo modprobe rbd
    rbd pool init rbdpool

    # Create Block Device Image
    rbd create rbdimage --size 1024 --pool rbdpool --image-feature layering
    # Use client.admin user. To use another user, replace parameter --name
    sudo rbd map rbdimage --pool rbdpool --name client.admin

    sudo mkfs.ext4 -m0 /dev/rbd/rbdpool/rbdimage
    sudo mkdir /mnt/extra/ceph-block-device
    sudo mount /dev/rbd/rbdpool/rbdimage /mnt/extra/ceph-block-device
    cd /mnt/extra/ceph-block-device

14. Benchmark Ceph (RBD)
    # Benchmark using RBD BENCH
        # Using rbd bench-write [DEPRECATED - NOT USED]
            # blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
            # for size in "${blockSizeArr[@]}"
            # do
            #     echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
            #     echo "$size"
            #     rbd bench-write rbdimage --pool=rbdpool --io-size $size --io-threads 1 --io-total 4G
            # done

        # https://docs.ceph.com/en/quincy/man/8/rbd/
        # bench –io-type <read | write | readwrite | rw> [–io-size size-in-B/K/M/G/T] [–io-threads num-ios-in-flight] [–io-total size-in-B/K/M/G/T] [–io-pattern seq | rand] [–rw-mix-read read proportion in readwrite] image-spec
        # Generate a series of IOs to the image and measure the IO throughput and latency. If no suffix is given, unit B is assumed for both –io-size and –io-total. Defaults are: –io-size 4096, –io-threads 16, –io-total 1G, –io-pattern seq, –rw-mix-read 50.

        # For sequential, write
        blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
        for size in "${blockSizeArr[@]}"
        do
            echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
            echo "$size"
            rbd bench --io-type write rbdimage --pool=rbdpool --io-size $size --io-threads 1 --io-total 4G --io-pattern seq
        done

        # For random, write
        blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
        for size in "${blockSizeArr[@]}"
        do
            echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
            echo "$size"
            rbd bench --io-type write rbdimage --pool=rbdpool --io-size $size --io-threads 1 --io-total 4G --io-pattern rand
        done

        # For sequential, read
        blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
        for size in "${blockSizeArr[@]}"
        do
            echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
            echo "$size"
            rbd bench --io-type read rbdimage --pool=rbdpool --io-size $size --io-threads 1 --io-total 4G --io-pattern seq
        done

        # For random, read
        blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
        for size in "${blockSizeArr[@]}"
        do
            echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
            echo "$size"
            rbd bench --io-type read rbdimage --pool=rbdpool --io-size $size --io-threads 1 --io-total 4G --io-pattern rand
        done

    # Benchmark using Bonnie++
        blockSizeArr=("4K" "8K" "16K" "32K" "64K" "128K" "256K" "512K" )
        val=1
        for size in "${blockSizeArr[@]}"
        do
            echo 3 | sudo tee /proc/sys/vm/drop_caches && sudo sync
            val=$(($val*2))
            valtemp="$val""K"
            echo "RAM $valtemp & BENCHFILE $size"
            bonnie++ -d /mnt/extra/ -r $valtemp -s $size -n 1 -f -b -u root:root -m BonnieRBD$size
        done