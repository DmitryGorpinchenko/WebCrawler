# WebCrawler 

## Usage
 
* to run this application, build it from the sources using Qt framework installed on your operation system

* in the *Start URL* text box enter a **valid** URL starting from *http://*

* in the *Query* text box enter some text you would like to find web pages with

* in the *URLs to scan* text box enter a **valid** integer number of URLs you **at maximum** want to scan before *web crawling* finishes

* press the *Start* button to launch the scanning process

* on the right area you should be able to observe the URLs already scanned and currently scanning with status indications
(**Loading ...** means that page is requested right now and program waits for the reply from the server, 
**FOUND** and **NOT FOUND** mean that the web page hosted on that particular URL contains or do not contain
the *Query*, respectively; **any other** status indicates that some error occured)

* *Abort* button could be used to prematurely finish scanning process

* *Pause/Resume* button could be used to pause and resume scanning of the web graph, respectively

* on the left you can observe the scanning progress via circular progress bar
