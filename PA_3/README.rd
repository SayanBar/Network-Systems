
Author: Sayan Barman 
Description: Distributed Server System
Four servers are run on four different ports and then clients are connected on each port to send and receive files in chunks.There is redundancy in the chunks, therefore we can retrieve all chunks if 1 or 2 non-consecutive servers are down.

Functionalities

GET- this function gets all the chunks from the servers that are available
GET <filename> 

PUT- this function splits the file and sends all the chunks to the servers
PUT <filename> <foldername(optional)>

LIST-This function lists the chunks available at different servers
LIST

To run the codes

To run webserver- ./dfs DFS<servernumber> <portnumber>

To run client-  ./dfc <userconffilename>


Features

SUbfolders for each user has been implemented

Subfolders under each user has also been implemented for PUT but not GET
 
