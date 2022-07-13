


========================================================================================
                            Run a Single Node CORTX-Motr
========================================================================================
# project ..849 @TACC "Skylake"
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
    
7. Create loop devices [Redo after restart]
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

    # mount loop devices

        mkdir -p /mnt/extra/loop-devs/loop0
        mkdir -p /mnt/extra/loop-devs/loop1
        mkdir -p /mnt/extra/loop-devs/loop2
        mkdir -p /mnt/extra/loop-devs/loop3
        cd /mnt/extra/loop-devs/
        sudo mount -o loop /dev/loop0 /mnt/extra/loop-devs/loop0
        sudo mount -o loop /dev/loop1 /mnt/extra/loop-devs/loop1
        sudo mount -o loop /dev/loop2 /mnt/extra/loop-devs/loop2
        sudo mount -o loop /dev/loop2 /mnt/extra/loop-devs/loop3
        lsblk -f
        df -h 

        # remove loop devs [No-NEED]
            # sudo umount /mnt/extra/loop-devs/loop0
            # sudo umount /mnt/extra/loop-devs/loop1
            # sudo umount /mnt/extra/loop-devs/loop2
            # sudo umount /mnt/extra/loop-devs/loop3
            # sudo losetup -d /dev/loop0
            # sudo losetup -d /dev/loop1
            # sudo losetup -d /dev/loop2
            # sudo losetup -d /dev/loop3
            # rm -rf /mnt/extra/loop-files/*.img

        # check using "lsblk"
            # we will use "loop4" "loop5" "loop6" for Motr 
            # "loop7" for log 

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
        sed -i '/loop7/d' CDF.yaml
        sed -i '/loop8/d' CDF.yaml

        # set the disk for logging
        sudo sed -i "s|loop9|loop7|g" CDF.yaml

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

    cd /mnt/extra 
    git clone https://daniarherikurniawan@github.com/daniarherikurniawan/cortx-bench-extra.git
        

12. Running Client 
    
    # example1.c 
        # export LD_LIBRARY_PATH=/mnt/extra/cortx-motr/motr/.libs/
        
        cd /mnt/extra/cortx-motr/motr/examples
        gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr example1.c -o example1
        ./example1 inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670

    # Benchmark

        blockSizeArr=("65536" "131072" "262144" "524288" "1048576")
        for blockSize in "${blockSizeArr[@]}"
        do
            echo "$blockSize"
            ./modify_param.py -file /mnt/extra/cortx-motr/motr/examples/example1.c -params "BLOCK_SIZE=$blockSize"
            gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr example1.c -o example1
            ./example1 inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670
        done


12. Run Multi Threaded Client [No-NEED]
    
    # Download example_mt_bench.c and analyze_lat.py

        cd /mnt/extra/cortx-motr/motr/examples/
        wget https://raw.githubusercontent.com/daniarherikurniawan/cortx-motr-all/master/motr/examples/example_mt_bench_latest.c
        wget https://raw.githubusercontent.com/daniarherikurniawan/cortx-motr-all/master/motr/examples/analyze_lat.py
            
        mv example_mt_bench_latest.c example_mt_bench.c
    
    # Build 
        cd /mnt/extra/cortx-motr/motr/examples/
        gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr -lpthread example_mt_bench.c -o example_mt_bench

    # Run 
        cd /mnt/extra/cortx-motr/motr/examples
        gcc -I/mnt/extra/cortx-motr -DM0_EXTERN=extern -DM0_INTERNAL= -Wno-attributes -L/mnt/extra/cortx-motr/motr/.libs -lmotr -lpthread example_mt_bench.c -o example_mt_bench
        ./example_mt_bench inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501 "<0x7000000000000001:0x0>" "<0x7200000000000001:0x3>" 12345670 1 1 1 3 1000 5
        

        ./example_mt_bench inet:tcp:10.140.81.147@22001 inet:tcp:10.140.81.147@22501  "<0x7000000000000001:0>" "<0x7200000000000001:64>" 12349741 1 0 0 10 1000 30
        
            # args : 1        0        0          10      1000       30 
                # <write?> <read?> <delete?> <layout_id> <n_req> <n_parallel>
    # Output 
        at /out_mt_bench/* 
            sub folder = <layout_id>-<n_req>-<n_parallel> 


            sudo ./analyze_lat.py -in_trace /mnt/extra/cortx-motr/motr/examples/out_mt_bench/3-1-1/93867_write_lat.txt >> /mnt/extra/cortx-motr/motr/examples/out_mt_bench/3-1-1/93867_stats.txt 
 


Next:
    - Test latency of 1000 request!
    - compare it to Faradawn's real storage_node deployment
    - Modify Motr,
        - recompile!
    - Check the logs!!























