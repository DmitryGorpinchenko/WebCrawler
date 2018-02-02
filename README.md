# WebCrawler 

## Usage assumptions
 
* to run this application, please, build it from the provided sources using the Qt framework installed in your operation system

* in the *Start URL* text box, please, enter a **valid** URL starting from *http://* (**NOT** *https://*)

* in the *Search text* text box, please, enter some text you would like to find web pages with

* in the *URLs to scan* text box, please, enter a **valid** integer number of URLs you **at maximum** want to scan before *web crawling* finishes

* once you fill in all these fields, press the *Start* button to launch the scanning process

* now on the right area you should be able to observe the URLs already scanned and currently scanning with status indications
(**Loading ...** means that page is requested right now and program waits for the reply from the server, 
**Text was FOUND** and **Text was NOT FOUND** mean that the web page hosted on that particular URL contains or do not contain 
the *Search text*, respectively; **any other** status indicates some error occured with the request to that URL)

* you can also press the *Stop* button to prematurely finish scanning the web graph

* also, on the left you can observe the scanning progress via circular progress bar

* please, notice that once you press the *Start*, you will not be able to interact with the GUI except with the *Stop* button;
only after the whole scanning completes or you press the *Stop*, the GUI becomes responding again

## Suggestions for testing this application

I suggest to create some artificial mini-web with several dozens, hundreds or even thousands of pages hosted on some
base URL with many links (set up with some underlying graph structure in advance) between pages and with text known in advance.
Then it would be possible to create manually some test cases and run the application on them and compare the expected sequence and 
status indicators of scanning process with that produced by the program (accounting, of course, for some error conditions that can occur on the web
and which we cannot in principle control). 

Also, of course, we can just manually test the functionality provided by the Start, Stop buttons and other GUI elements or I think we could even simulate
their functionality programatically (i.e. without user intervention) to be able to test our program for resistence under the stream of simulated 'button click' 
events and monitor that nothing bad happen during such execution.

Finally, dispite the fact that Qt is a cross-platform framework, I think that it would be worthwhile to test this application 
on different target operating systems to be completely sure that it works as expected.
