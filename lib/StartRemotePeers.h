#ifndef _start_remote_peers_h
#define _start_remote_peers_h

/*
 *                     CNT4007 Project
 * This is the program starting remote processes.
 * This program was only tested on CISE SunOS environment.
 * If you use another environment, for example, linux environment in CISE 
 * or other environments not in CISE, it is not guaranteed to work properly.
 * It is your responsibility to adapt this program to your running environment.
 */
 
 //NOTE: modified from java code
/*
public class RemotePeerInfo {
	public String peerId;
	public String peerAddress;
	public String peerPort;
	
	public RemotePeerInfo(String pId, String pAddress, String pPort) {
		peerId = pId;
		peerAddress = pAddress;
		peerPort = pPort;
	}
}
*/

void start_remote_peer(peer_info pi);

#endif