#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

// ============================================================================
void decToBinary(int n, char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}



// ============================================================================

class FsFile {
    int file_size;
    int block_in_use;
    int index_block;
    int block_size;

public:
     FsFile(int _block_size) {
        file_size = 0;
        block_in_use = 0;
        block_size = _block_size;
        index_block = -1;
    }

    int getfile_size(){
        return file_size;
    }
    void setIndexBlock(int index){
        this->index_block = index;
    }
    int getIndexBlock(){
        return this->index_block;
     }
  
};

// ============================================================================

class FileDescriptor {
    string file_name;
    FsFile* fs_file;
    bool inUse;

public:

    FileDescriptor(string FileName, FsFile* fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }

    string getFileName() {
        return file_name;
    }
    bool GetInUse() {
        return inUse;
    }
    void setInUse(bool inUse) {
        this->inUse=inUse;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

// ============================================================================

class fsDisk {
    FILE *sim_disk_fd;

    bool is_formated;

    int BitVectorSize;
    int *BitVector;

    // MainDir -- Structure that links the file name to its FsFile
    map<int,FileDescriptor*> MainDir;// <int,FileDescriptor>

     /*OpenFileDescriptors -- when you open a file,
     the operating system creates an entry to represent that file
     This entry number is the file descriptor.*/
    map<int,FileDescriptor*> OpenFileDescriptors;

    int getNextFd(){
        return this->MainDir.size();// change after delete
    }
    // ------------------------------------------------------------------------
public:
    fsDisk() {
        sim_disk_fd = fopen(DISK_SIM_FILE , "r+");
        assert(sim_disk_fd);

        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd);
            assert(ret_val == 1);
        }

        fflush(sim_disk_fd);
        is_formated = false;
    }

    // ------------------------------------------------------------------------
    void listAll() {
        int i = 0;

        for (auto j = MainDir.begin(); j != MainDir.end();j++) { //somting wong with zis
            cout << "index: " << i << ": FileName: " << j->first <<  " , isInUse: " << j->second->GetInUse() << endl;
            i++;
        }

        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            cout << "(";
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
            cout << ")";
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat( int blockSize =4 ) {

        // 1. set Bit-Vector
        //
        this->BitVectorSize = DISK_SIZE / blockSize;
        this->BitVector = new int[this->BitVectorSize];

        // 2. Fill Bit-Victor with 0's
        for (int i = 0; i < this->BitVectorSize ; ++i) {
            BitVector[i]=0;
        }
        this->is_formated = true;

        cout << "FORMAT DISK: number of blocks: " << BitVectorSize << endl;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {

        int blockSize = DISK_SIZE / this->BitVectorSize;

        FsFile* newFs =new FsFile(blockSize);

        // 1. find empty block
        int i;
        for (i = 0; i <this->BitVectorSize ; ++i) {
            if(this->BitVector[i]==0)
            {
                // 2. create index block
                newFs->setIndexBlock(i);
                this->BitVector[i]=1;
                break;
            }
        }

        int fd =getNextFd(); // wisa cont

        FileDescriptor *File = new FileDescriptor(fileName,newFs);

        this->MainDir.insert({fd,File});

        this->OpenFileDescriptors.insert({fd,File});

        return i;
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) {

        if(!this->is_formated) { // the file is not created
            return  -1;
        }

        for (auto i = this->MainDir.begin();i !=this->MainDir.end(); ++i) {
            if (fileName == i->second->getFileName()) {
                if(i->second->GetInUse()){
                cout << "File is already OPEN" << endl;
                return i->first;
                }
                else {
                    i->second->setInUse(true);
                    this->OpenFileDescriptors.insert({i->first, i->second});
                    return i->first;
                }
            }
        }

    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {

      FileDescriptor *File = OpenFileDescriptors.at(fd);
      if(File==NULL) {
          cout << "cannot close file" << endl;
          return "-1";
      }
      File->setInUse(false);
      OpenFileDescriptors.erase(fd);
        return File->getFileName();

    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len ) {

        int blockSize = DISK_SIZE / this->BitVectorSize;


    }
    // ------------------------------------------------------------------------
    int DelFile( string FileName ) {

    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len ) {


    }
};

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;


    fsDisk *fs = new fsDisk();
    int cmd_;
    while(true) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            default:
                break;
        }
    }
}