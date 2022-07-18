

========================================================================================
                        Run a Single Node CORTX-Motr (Loops)
========================================================================================

# project ..849 @UC "Skylake"
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
        
        sudo ./configure --disable-expensive-checks --disable-immediate-trace  --disable-dev-mode --with-trace-max-level=M0_ERROR
        
        # ./configure --with-trace-max-level=M0_INFO
        # ./configure --help | grep trace

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
    
    cd /mnt/extra/py-utils/dist
    sudo yum install -y cortx-py-utils-*.noarch.rpm

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

    # add path to zsh
    echo 'PATH=/opt/seagate/cortx/hare/bin:$PATH' >> ~/.zshrc
    echo 'export LD_LIBRARY_PATH=/mnt/extra/cortx-motr/motr/.libs/' >> ~/.zshrc
    source ~/.zshrc
    
7. Create loop devices [[AFTER EACH REBOOT]]
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

        # sample output: 10.140.81.147@tcp

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

    # bootstrap (0.5 min)
        cd /mnt/extra
        sudo hctl bootstrap --mkfs CDF.yaml
        hctl status
        
        # cd /mnt/extra
        # sudo hctl shutdown; sudo hctl bootstrap --mkfs CDF.yaml

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
                [started]  hax                 0x7200000000000001:0x0          inet:tcp:10.140.81.147@22001
                [started]  confd               0x7200000000000001:0x1          inet:tcp:10.140.81.147@21002
                [started]  ioservice           0x7200000000000001:0x2          inet:tcp:10.140.81.147@21003
                [unknown]  m0_client_other     0x7200000000000001:0x3          inet:tcp:10.140.81.147@22501
                [unknown]  m0_client_other     0x7200000000000001:0x4          inet:tcp:10.140.81.147@22502

10. Connect VSCode ssh 
    # Use the "remote explorer" package to establish remote editor
    # Open the /mnt/extra 
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
    
    # copy Motr sample client 
    cd /mnt/extra/
    cp cortx-bench-extra/motr-clients/*  cortx-motr/motr/examples/
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
        ./example1 inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670

    # example1_dan.c 

        # Single benchmark 

            #!/usr/bin/env bash

            ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_dan.c -params "N_REQUEST=2000 BLOCK_SIZE=16777216"
            gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr example1_dan.c -o example1_dan
            ./example1_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670

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
                ./example1_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670
                rm -rf m0trace*     # don't run this line if you will use ADDB
            done


12. Run Multi Threaded Client [USE-THIS]
    
    # Write/Read/Delete
    #   1  /  1 /   1

    
    cd /mnt/extra/cortx-motr/motr/examples
    ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_multithd_dan.c -params "N_REQUEST=1000 BLOCK_SIZE=16777216 N_PARALLEL_THD=1"
    gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr -lpthread example1_multithd_dan.c -o example1_multithd_dan
    
    ./example1_multithd_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 1 0 0 
    ./example1_multithd_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 1 0 
    ./example1_multithd_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 0 1 

    # Benchmark type-A
        # "4096" "8192" "16384" "32768" "65536" "131072" "262144" "524288" "1048576" "2097152" "4194304" "8388608" "16777216"

        cd /mnt/extra/cortx-motr/motr/examples
        blockSizeArr=("2097152" "4194304" "8388608" "16777216")
        for blockSize in "${blockSizeArr[@]}"
        do
            echo "$blockSize"
            ./script/modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1_multithd_dan.c -params "N_REQUEST=20000 BLOCK_SIZE=$blockSize"
            gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr -lpthread example1_multithd_dan.c -o example1_multithd_dan
            ./example1_multithd_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 1 0 0 
            ./example1_multithd_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 1 0 
            ./example1_multithd_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 0 0 1 
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
            ./example1_multithd_dan inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 1 1 1
            rm -rf m0trace*     # don't run this line if you will use ADDB
        done







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


2. Install Kubernetes
    
    sudo mkdir -p /mnt/extra
    sudo chown daniar -R /mnt
    cd /mnt/extra


    # edit the hosts' IP
    sudo bash -c "echo 10.140.81.147 `hostname` >> /etc/hosts"


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
        echo -e "export KUBECONFIG=/etc/kubernetes/admin.conf \nalias kc=kubectl \nalias all=\"kubectl get pods -A -o wide\"" >> /etc/bashrc && source /etc/bashrc
        kubectl create -f https://projectcalico.docs.tigera.io/manifests/tigera-operator.yaml
        cd /mnt/extra
        wget https://gist.githubusercontent.com/faradawn/2288618db8ad0059968f48b6647732f9/raw/133f7f5113b4bc76f06dd5240ae7775c2fb74307/custom-resource.yaml
        kubectl create -f custom-resource.yaml
        exit 

        # turn off kubeadm
            # sudo kubeadm reset

3. Deploy CORTX to kubernetes
    
    # clone k8s repo and download solution.example.yaml
    cd /mnt/extra
    git clone -b main https://github.com/Seagate/cortx-k8s

    # modify solution.yaml 
        cd /mnt/extra/cortx-k8s/k8_cortx_cloud
        sudo sed -i 's|csm_mgmt_admin_secret: null|csm_mgmt_admin_secret: Cortx123|g' solution.example.yaml
        
        vim solution.example.yaml

            # 1. Search for "?nodes"
                # Change the nodes -> make sure it points to the desired nodes 
                # for 1 node deployment, the node1: name should be `hostname`
                # then delete other nodes 

            # 2. Search for "?sd"
                # we will use "dev/loop4" "dev/loop5" "dev/loop6" "dev/loop7" for Motr 
                # make sure that those devices existed, otherwise follow "Create loop devices"
                # Changes the sdc to loop4 and so on!
                    # svg1 uses "dev/loop4" "dev/loop5"
                    # svg2 uses "dev/loop6" "dev/loop7"
                    # size is 25Gi 
                    # remove other sd* 

        mv solution.example.yaml solution.yaml
    

    cd /mnt/extra/cortx-k8s/k8_cortx_cloud
    sudo ./prereq-deploy-cortx-cloud.sh -d /dev/loop8 -s solution.yaml

    sudo su
        # untaint master 
            # change the dan-cortx-1.novalocal with the output of `hostname`
            kubectl taint node dan-cortx-1.novalocal node-role.kubernetes.io/master:NoSchedule-
            kubectl taint node dan-cortx-1.novalocal node-role.kubernetes.io/control-plane:NoSchedule-

        # restart core-dns pods
        kubectl rollout restart -n kube-system deployment/coredns

        # start deploy
        tmux new -t k8s
            # session not found: k8s
        time ./deploy-cortx-cloud.sh solution.example.yaml

        ctl b d
        tmux a -t k8s
    exit

4. Upload File to CORTX-K8 (IAM API)
    # login to CSM to get the Bearer token 
    export CSM_IP=`kubectl get svc cortx-control-loadbal-svc -ojsonpath='{.spec.clusterIP}'`
        # Error from server (NotFound): services "cortx-control-loadbal-svc" not found

    kubectl get secrets/cortx-secret --namespace default --template={{.data.csm_mgmt_admin_secret}} | base64 -d

    tok=$(curl -d '{"username": "cortxadmin", "password": "Cortx123!"}' https://$CSM_IP:8081/api/v2/login -k -i | grep -Po '(?<=Authorization: )\w* \w*') && echo $tok

    # create and check IAM user
    curl -X POST -H "Authorization: $tok" -d '{ "uid": "12345678", "display_name": "gts3account", "access_key": "gregoryaccesskey", "secret_key": "gregorysecretkey" }' https://$CSM_IP:8081/api/v2/iam/users -k

    curl -H "Authorization: $tok" https://$CSM_IP:8081/api/v2/iam/users/12345678 -k -i

    # install aws [optional]
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


5. Benchmark CORTX-K8
    
    cd /mnt/extra
    sudo yum install -y go
    wget https://github.com/Seagate/s3bench/releases/download/v2022-03-14/s3bench.2022-03-14 && chmod +x s3bench.2022-03-14 && mv s3bench.2022-03-14 s3bench

    ./s3bench -accessKey gregoryaccesskey -accessSecret gregorysecretkey -bucket loadgen -endpoint http://$IP:$PORT -numClients 5 -numSamples 100 -objectNamePrefix=loadgen -objectSize 1Mb -region us-east-1 -o test1.log






Next:
    - Test latency of 1000 request!
    - compare it to Faradawn's real storage_node deployment
    - Modify Motr,
        - recompile!
    - Check the logs!!























