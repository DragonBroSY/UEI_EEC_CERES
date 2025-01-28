#include <iostream>
#include "UeiDaq.h"
#include <cstdlib> 
#include "custom_init.h"

using namespace UeiDaq;
CUeiSession mySession;
int* data;
float* inVals;

// This example continuously sends scheduled ARINC-429 words on TX port 0 and receives on port RX 0 and 1
int main(int argc, char* argv[])
{
    int i, ch, f;
    int numInputPorts;
    int numOutputPorts;
    int numWords = 12;
    data = (int*)malloc(sizeof(int) * 4);

    float EEC_UEI_test1; //input params from shmem
    float EEC_UEI_test2; //input params from shmem

    custom_init customInst;
    float* shmptr = customInst.comm_init();

    try
    {
        mySession.CreateARINCInputPort("pdna://192.6.1.36/dev0/arx0,1", 
                                        UeiARINCBitsPerSecond12500,
                                        UeiARINCParityNone,
                                        false,
                                        0);
        numInputPorts = mySession.GetNumberOfChannels();

        mySession.CreateARINCOutputPort("pdna://192.6.1.36/dev0/atx0", 
                                         UeiARINCBitsPerSecond12500,
                                         UeiARINCParityNone);
        numOutputPorts = mySession.GetNumberOfChannels()-numInputPorts;

        mySession.ConfigureTimingForMessagingIO(1, 0);

        // Set timeout to a very low value to avoid blocking the program
        // if no words are received
        mySession.GetTiming()->SetTimeout(10);

        // Allocate one reader for each configured input port
        CUeiARINCReader** readers = new CUeiARINCReader*[numInputPorts];

        typedef struct _tUeiARINCFilterEntry
        {
            // Label to accept
            uInt32 Label;
            // Accept word only if it carries different data from a
            // previously received word with the same label.
            Int32 NewData;
            // Trigger the scheduler entry with the same index as this
            // filter entry in the scheduler table of the output port
            // that has the same index as this input port.
            Int32 TriggerSchedulerEntry;
        } tUeiARINCFilterEntry;

        // Add an entry to filter
        tUeiARINCFilterEntry entry = { 12, 1, 1 };

        for(ch=0; ch<numInputPorts; ch++)
        {
            readers[ch] = new CUeiARINCReader(mySession.GetDataStream(), mySession.GetChannel(ch)->GetIndex());
            // Get pointer to the input channel
            CUeiARINCInputPort* pPort =
            dynamic_cast<CUeiARINCInputPort*>(mySession.GetChannel(ch));
            //pPort->AddFilterEntry(entry);
        }

        CUeiARINCWriter** writers = new CUeiARINCWriter*[numOutputPorts];
        for(ch=0; ch<numOutputPorts; ch++)
        {
            writers[ch] = new CUeiARINCWriter(mySession.GetDataStream(), mySession.GetChannel(ch)->GetIndex());
        }

        // Queue a few frames in each output port scheduler
        for(ch=0; ch<numOutputPorts; ch++)
        {
            // Get a pointer to each output port object
            CUeiARINCOutputPort* pOutPort = dynamic_cast<CUeiARINCOutputPort*>(mySession.GetChannel(numInputPorts + ch));

            for(f=0; f<numWords; f++)
            {
                tUeiARINCSchedulerEntry schedEntry;

                // Every fourth entry is a master.
                // All slave entries following the master entry (until the next master entry) 
                // are scheduled at the same time
                schedEntry.Master = (0 == (f % 4));

                // Master entry is periodic (this is ignored for slave entries)
                schedEntry.Periodic = 1;

                // Schedule master entry to output word 100ms after trigger
                schedEntry.Delay = 100000;

                // Program word to be emitted by this entry
                schedEntry.Word.Label = f + ch;
                schedEntry.Word.Sdi = 1;
                schedEntry.Word.Ssm = 2;
                schedEntry.Word.Data = 2*f;

                pOutPort->AddSchedulerEntry(schedEntry);
            }

            pOutPort->EnableScheduler(true);
        }

        // Allocate buffer used to receive ARINC words
        tUeiARINCWord* words = new tUeiARINCWord[numWords];

        mySession.Start();

        for(;;)
        {
            for(ch=0; ch<numInputPorts; ch++)
            {
                int port = mySession.GetChannel(ch)->GetIndex();
                Int32 numWordsRead;

                // Wrap Read in its own try/catch block to filter out 
                // timeout error
                try
                {
                    readers[ch]->Read(numWords, words, &numWordsRead);
                }
                catch(CUeiException& e)
                {
                    if(e.GetError() == UEIDAQ_TIMEOUT_ERROR)
                    {
                        continue;
                    }

                    // re-throw any other error, the main exception handler
                    // will process it
                    throw;
                }

                for(f=0; f<numWordsRead; f++)
                {
                    std::cout << std::dec << i << "  RX Port " << port << ": Received word :" << 
                        " Label=" << std::hex << words[f].Label << std::dec << 
                        " Data=" << words[f].Data << 
                        " Sdi=" << words[f].Sdi <<
                        " Ssm=" << words[f].Ssm <<
                        " Parity=" << words[f].Parity << std::endl;

                    shmptr = customInst.update(data);
                    /* wait 5 us */
                    usleep(5000);
                    std::cout << "shmptr OUTSIDE comm_init() is:" << shmptr << "\n";

                    EEC_UEI_test1 = customInst.inVal1;
                    EEC_UEI_test2 = customInst.inVal2;
                    std::cout << "Read in params EEC_UEI_test1&2: " << shmptr[0] << " " << shmptr[1] << "\n";

                    usleep(50000);

                    data[0] = words[f].Label;
                    data[1] = words[f].Data;
                    data[2] = words[f].Sdi;
                    data[3] = words[f].Ssm;

                    std::cout << "Read back value from wrote to shmptr[0], shmptr[1], shmptr[2], shmptr[3]:" << shmptr[0] << " " << shmptr[1] << " " << shmptr[2] << " " << shmptr[3] << '\n';

                }
            }

            // update scheduled words after 50 iterations
            /*if(50 == i) 
            {
               for(ch=0; ch<numOutputPorts; ch++)
               {
                  tUeiARINCWord* words = new tUeiARINCWord[numWords];
                  int numWordsWritten;
                  int port = mySession.GetChannel(ch)->GetIndex();

                  for(f=0; f<numWords; f++)
                  {
                     words[f].Label = f;
                     words[f].Ssm = 2;
                     words[f].Sdi = 1;
                     words[f].Data = 102+f;

                     std::cout << std::dec << i << "  TX Port " << port << ": TX'd word :" <<
                         " Label=" << std::hex << words[f].Label << std::dec <<
                         " Data=" << words[f].Data <<
                         " Sdi=" << words[f].Sdi <<
                         " Ssm=" << words[f].Ssm <<
                         " Parity=" << words[f].Parity << std::endl;
                  }

                  writers[ch]->WriteScheduler(0, numWords, words, &numWordsWritten);

                  std::cout << "Updated " << numWordsWritten << " scheduled words" << std::endl;

                  delete[] words;
               }
            }*/
        }

        mySession.Stop();

        for(ch=0; ch<numInputPorts; ch++)
        {
            delete readers[ch];
        }
        delete[] readers;
        
        delete[] words;

        mySession.CleanUp();
    }
    catch(CUeiException& e)
    {
        std::cout << "Error " << std::hex << e.GetError() << ": " << e.GetErrorMessage() << std::endl;
    }

    return 0;
}