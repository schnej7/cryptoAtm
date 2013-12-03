Downloads Required:
c++ boost: libboost-all-dev

Download cryptopp Libraries:
apt-cache pkgnames | grep -i crypto++
apt-get install libcrypto++9 libcrypto++9-dbg libcrypto++-dev

compiler flags (really you should just use the make file):

bank, proxy: -lpthread
bank, atm, util: -lcryptopp

Protocol:

First, once the atm program is started it will ask for an atm number which must be [1-10]
to determine which atm is currently being used. Then the atm performs a handshake procedure
with the bank in which the atm sends a nonce encrypted with AES using its local session key.
Then the bank uses each of its stored possible session keys attempt to decrypt the received
package. When the bank is successful it then knows that the session key it used to decrypt
the message is the session key for that paticular atm.  The bank sends a success signal back
to the atm along with a continuation of the nonce passing (the atms original nonce along with
a new nonce).  After the hand shake the atm can send messages to the bank to request various
pieces of information or transactions and the bank will respond with the appropiate output.
The messages used by both the bank and the atm when they are passed are of the following
form: <IV> Encrypted[ <atmNonce> <message> <bankNonce> ] where the encryption is done using AES.