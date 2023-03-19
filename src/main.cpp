#include <iostream>
#define P2P_IMPLEMENTATION
#include "p2p.h"

    

int main() {

    p2p_init();

    p2p_socket* s = p2p_create_socket("stun.l.google.com", 19302, false);
    std::cout << "id: \n" << s->sdp << std::endl;
    while(true) {
        if(s->is_connected) {
            std::string str;
            std::cin >> str;
            p2p_send_raw(s, str.c_str(), str.size());
        }
        else if(s->has_gathered) {
            std::cout << "input remote: " << std::endl;
            std::string peer;
            std::cin >> peer;
            p2p_connect(s, peer.c_str());
            while(s->in_negotiation) {
                std::this_thread::sleep_for(std::chrono::duration<float>(1.0f));
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::duration<float>(1.0f));
        }
    }

    p2p_destroy_socket(s);
	return 0;
}


