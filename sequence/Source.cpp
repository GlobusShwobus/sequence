#include "Sequence.h"
#include "Stopwatch.h"
#include <iostream>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  
#include <crtdbg.h>  
#include <conio.h>
#include <vector>

struct Rect {
    int x;
    int y;
    int w;
    int h;
    Rect() = default;
    Rect(int x, int y, int w, int h) :x(x), y(y), w(w), h(h) {}
    Rect(const Rect& rhs) {
        x = rhs.x;
        y = rhs.y;
        w = rhs.w;
        h = rhs.h;
    }
    Rect& operator=(const Rect& rhs) {
        x = rhs.x;
        y = rhs.y;
        w = rhs.w;
        h = rhs.h;
        return *this;
    }
    Rect(Rect&& rhs)noexcept {
        x = rhs.x;
        y = rhs.y;
        w = rhs.w;
        h = rhs.h;
        rhs.x = -1;
        rhs.y = -1;
        rhs.w = -1;
        rhs.h = -1;
    }
    Rect& operator=(Rect&& rhs)noexcept {
        x = rhs.x;
        y = rhs.y;
        w = rhs.w;
        h = rhs.h;
        rhs.x = -1;
        rhs.y = -1;
        rhs.w = -1;
        rhs.h = -1;
        return *this;
    }
};

void round1Tests_bullshits() {
    Sequence<int> myInts;

    for (int i = 0; i < 99; i++) {
        myInts.add(i * i);
    }

    for (int o : myInts) {
        std::cout << o << '\n';
    }
    std::cout << "\n\n";
    std::cout << myInts.size() << " << MY SIZE\n";
    std::cout << myInts.capacity() << " << MY CAP";

    myInts.clear();

    std::cout << "\n\n";
    std::cout << myInts.size() << " << MY SIZE\n";
    std::cout << myInts.capacity() << " << MY CAP";

    //myInts.pop_back(); // crashes here == good

    myInts.clear();//should not crash on empty

    for (int i = 0; i < 99; i++) {
        myInts.add(i * i);
    }

    std::cout << "\n\n";
    std::cout << myInts.size() << " << MY SIZE\n";
    std::cout << myInts.capacity() << " << MY CAP";

    for (int i = 0; i < 27; i++) {
        myInts.pop_back();
    }

    std::cout << "\n\n";
    std::cout << myInts.size() << " << MY SIZE\n";
    std::cout << myInts.capacity() << " << MY CAP";

    std::cout << "\n\n";
    std::cout << myInts[10];

    std::cout << "\n\n";
    // std::cout << myInts[myInts.size()];//should crash
    std::cout << myInts[myInts.size() - 1];

    const Sequence<int> copy = myInts;
    Sequence<int> copy2;
    copy2.add(5);

    copy2 = copy;

    std::cout << "\n\n";
    std::cout << "COPY\n";
    auto cit = copy.begin();//const iteragor - good
    //int& lol = cit[2];//not allowed since cosnt
    int lol = cit[copy.size() - 1];
    std::cout << lol << "my LOL\n";

    int lol2 = cit[copy2.size() - 1];
    std::cout << lol2 << "my LOL2\n";
}

void round2Tests_testing_iterator_operators() {
    Sequence<int> seq;
    std::vector<int> vec;
    
    for (int i = 0; i < 100; i++) {
        seq.add(i * i);
        vec.push_back(i * i);
    }

    auto seqIt = seq.begin();
    auto vecIt = vec.begin();

    std::cout << "TEST operator*\t\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';

    ++seqIt;
    ++vecIt;

    std::cout << "TEST operator++\t\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';
   
    ++seqIt;
    ++vecIt;
    ++seqIt;
    ++vecIt;
    ++seqIt;
    ++vecIt;
    ++seqIt;
    ++vecIt;
   
    std::cout << "TEST operator++ x4\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';

    --seqIt;
    --vecIt;

    std::cout << "TEST operator--\t\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';


    std::cout << "\n\nPOST INCREMENT TEST:\n";

    std::cout << "INITIAL\t\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';
    std::cout << "POST INCR\tseq:" << *seqIt++ << "\t\tvec:" << *vecIt++ << '\n';
    std::cout << "AFTER\t\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';

    std::cout << "\n\nTESTING OPERATOR -> on basic struct:\n";

    struct Ass {
        int x = 69;
        int y = 420;
    };

    Sequence<Ass> assesSeq;
    std::vector<Ass> assesVec;
    assesSeq.add({});
    assesVec.push_back({});
    auto assesSeqIt = assesSeq.begin();
    auto assesVecIt = assesVec.begin();

    std::cout << "TEST operator-> x\tseq:" << assesSeqIt->x << "\t\tvec:" << assesVecIt->x << '\n';
    std::cout << "TEST operator-> y\tseq:" << assesSeqIt->y << "\t\tvec:" << assesVecIt->y << '\n';


    std::cout << "\n\n";
    seqIt += 5;
    vecIt += 5;

    std::cout << "TEST operator+=\t\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';

    seqIt -= 2;
    vecIt -= 2;

    std::cout << "TEST operator-=\t\tseq:" << *seqIt << "\t\tvec:" << *vecIt << '\n';

    auto seqIt2 = seqIt + 3;
    auto vecIt2 = vecIt + 3;

    std::cout << "TEST operator+\t\tseq:" << *seqIt2 << "\t\tvec:" << *vecIt2 << '\n';

    auto seqIt3 = seqIt2 - 5;
    auto vecIt3 = vecIt2 - 5;

    std::cout << "TEST operator-\t\tseq:" << *seqIt3 << "\t\tvec:" << *vecIt3 << '\n';

    auto seqDist = seqIt3 - seqIt;
    auto vecDist = vecIt3 - vecIt;

    std::cout << "TEST operator- dist\tseq:" << seqDist << "\t\tvec:" << vecDist << '\n';

    int seqVal = seqIt[7];
    int vecVal = vecIt[7];
    std::cout << "TEST operator[]\t\tseq:" << seqVal << "\t\tvec:" << vecVal << '\n';

    int seqVal2 = seqIt[19] - seqIt[7];
    int vecVal2 = vecIt[19] - vecIt[7];

    std::cout << "TEST operator[]+[]\tseq:" << seqVal2 << "\t\tvec:" << vecVal2 << '\n';

    auto seqBools = seq.begin() + 15;
    auto seqBools2 = seq.begin() + 15;

    auto vecBools = vec.begin() + 15;
    auto vecBools2 = vec.begin() + 15;

    std::cout << "\n\nTESTING BOOLEAN OPERATIONS:\nSAME POSITIONS\n";

    bool seqSame = seqBools == seqBools2;
    bool vecSame = vecBools == vecBools2;

    std::cout << "TEST operator==\t\tseq:" << seqSame << "\t\tvec:" << vecSame << "\n";

    seqSame = seqBools != seqBools2;
    vecSame = vecBools != vecBools2;
    std::cout << "TEST operator!=\t\tseq:" << seqSame << "\t\tvec:" << vecSame << "\n";

    std::cout << "\nINCREMENT ONE OF THE ITERATORS:\n";
    ++seqBools2;
    ++vecBools2;

    seqSame = seqBools == seqBools2;
    vecSame = vecBools == vecBools2;
    std::cout << "TEST operator==\t\tseq:" << seqSame << "\t\tvec:" << vecSame << "\n";

    seqSame = seqBools != seqBools2;
    vecSame = vecBools != vecBools2;

    std::cout << "TEST operator!=\t\tseq:" << seqSame << "\t\tvec:" << vecSame << "\n";

    std::cout << "\nIS UNINCREMENTED LESS THAN INCREMENTED:\n";
    bool seqLess = seqBools < seqBools2;
    bool vecLess = vecBools < vecBools2;

    std::cout << "TEST operator<\t\tseq:" << seqLess << "\t\tvec:" << vecLess << "\n";
    std::cout << "\nIS UNINCREMENTED MORE THAN INCREMENTED:\n";
    seqLess = seqBools > seqBools2;
    vecLess = vecBools > vecBools2;
    std::cout << "TEST operator>\t\tseq:" << seqLess << "\t\tvec:" << vecLess << "\n";

    std::cout << "\nDECREMENT SECOND TWICE:\n";
    --seqBools2;
    --vecBools2;
    --seqBools2;
    --vecBools2;

    std::cout << "\nIS UNINCREMENTED LESS THAN DECREMENTED:\n";
    seqLess = seqBools < seqBools2;
    vecLess = vecBools < vecBools2;

    std::cout << "TEST operator<\t\tseq:" << seqLess << "\t\tvec:" << vecLess << "\n";
    std::cout << "\nIS UNINCREMENTED MORE THAN DECREMENTED:\n";
    seqLess = seqBools > seqBools2;
    vecLess = vecBools > vecBools2;
    std::cout << "TEST operator>\t\tseq:" << seqLess << "\t\tvec:" << vecLess << "\n";

    std::cout << "\nDECREMENT SECOND ONCE:\n";
    ++seqBools2;
    ++vecBools2;

    std::cout << "\nIS UNINCREMENTED LESS THAN OR EQUALS TO DECREMENTED:\n";
    seqLess = seqBools <= seqBools2;
    vecLess = vecBools <= vecBools2;

    std::cout << "TEST operator<=\t\tseq:" << seqLess << "\t\tvec:" << vecLess << "\n";
    std::cout << "\nIS UNINCREMENTED MORE THAN OR EQUALS TODECREMENTED:\n";
    seqLess = seqBools >= seqBools2;
    vecLess = vecBools >= vecBools2;
    std::cout << "TEST operator>=\t\tseq:" << seqLess << "\t\tvec:" << vecLess << "\n";

    std::cout << "\nIF ALL VALUES ARE EXACTLY THE SAME, THEN SUCCESS:\n";
}

void round3Tests_sequence_adding_erasing() {

    Sequence<Rect> seq;
    std::vector<Rect> vec;

    std::cout << "INITIAL isEmpty:seq: " << seq.isEmpty() << "\tvec: " << vec.empty() << '\n';
    std::cout << "INITIAL SIZE:\tseq: " << seq.size() << "\tvec: " << vec.size()<<'\n';
    std::cout << "INITIAL CAP: \tseq: " << seq.capacity() << "\tvec: " << vec.capacity() << '\n';


    for (int i = 0; i < 100; i++) {
        Rect r{ i,i + 1,i + 2,i + 3 };
        seq.add(r);
        vec.push_back(r);
    }
    std::cout << "\n\n";
    std::cout << "AFTER isEmpty:\tseq: " << seq.isEmpty() << "\tvec: " << vec.empty() << '\n';
    std::cout << "AFTER SIZE:\tseq: " << seq.size() << "\tvec: " << vec.size() << '\n';
    std::cout << "AFTER CAP: \tseq: " << seq.capacity() << "\tvec: " << vec.capacity() << '\n';

    Rect copyRect = { 50,50,50,50 };

    seq.add(copyRect);
    vec.push_back(copyRect);

    auto seqIt = seq.end() - 1;
    auto vecIt = vec.end() - 1;
    std::cout << "\n\nCOPYING A RECTANGLE TO THE END:\n";
    std::cout << "SEQUENCE: " << seqIt->x << " " << seqIt->y << " " << seqIt->w << " " << seqIt->h << "\n";
    std::cout << "VECTOR: " << vecIt->x << " " << vecIt->y << " " << vecIt->w << " " << vecIt->h << "\n";

    Rect moveRect = { 69,69,69,69 };
    seq.add(std::move(moveRect));
    vec.push_back(std::move(moveRect));
    auto seqIt2 = seq.end() - 1;
    auto vecIt2 = vec.end() - 1;

    std::cout << "\nMOVING A RECTANGLE TO THE END - > VECTOR HAS TO BE JUNK TO SUCCEED\n";
    std::cout << "SEQUENCE: " << seqIt2->x << " " << seqIt2->y << " " << seqIt2->w << " " << seqIt2->h << "\n";
    std::cout << "VECTOR: " << vecIt2->x << " " << vecIt2->y << " " << vecIt2->w << " " << vecIt2->h << "\n";

    Rect moveRect2 = { 420,420,420,420 };
    vec.push_back(std::move(moveRect2));
    seq.add(std::move(moveRect2));

    auto seqIt3 = seq.end() - 1;
    auto vecIt3 = vec.end() - 1;
    std::cout << "\nMOVING A RECTANGLE TO THE END - > SEQUENCE HAS TO BE JUNK TO SUCCEED\n";
    std::cout << "SEQUENCE: " << seqIt3->x << " " << seqIt3->y << " " << seqIt3->w << " " << seqIt3->h << "\n";
    std::cout << "VECTOR: " << vecIt3->x << " " << vecIt3->y << " " << vecIt3->w << " " << vecIt3->h << "\n";

    Rect rect3 = { 222,222,222,222 };
    Rect rect4 = { 333,333,333,333 };
    seq.add(rect3);
    seq.add(rect4);
    vec.push_back(rect3);
    vec.push_back(rect4);

    std::cout << "\nTESTING POP BACK:\n";

    {
        std::cout << "\nBEFORE POP BACK:\n";
        Rect& recSeq = seq[seq.size() - 1];
        Rect& reqVec = vec[vec.size() - 1];
        std::cout << "SEQUENCE: " << recSeq.x << " " << recSeq.y << " " << recSeq.w << " " << recSeq.h << "\n";
        std::cout << "VECTOR: " << reqVec.x << " " << reqVec.y << " " << reqVec.w << " " << reqVec.h << "\n";
    }
    {
        seq.pop_back();
        Rect& recSeq2 = seq[seq.size() - 1];
        Rect& reqVec2 = vec[vec.size() - 1];
        std::cout << "\nPOP BACK SEQUENCE:\n";
        std::cout << "SEQUENCE: " << recSeq2.x << " " << recSeq2.y << " " << recSeq2.w << " " << recSeq2.h << "\n";
        std::cout << "VECTOR: " << reqVec2.x << " " << reqVec2.y << " " << reqVec2.w << " " << reqVec2.h << "\n";
    }
    {
        vec.pop_back();
        std::cout << "\nPOP BACK VECTOR:\n";
        Rect& recSeq3 = seq[seq.size() - 1];
        Rect& reqVec4 = vec[vec.size() - 1];
        std::cout << "SEQUENCE: " << recSeq3.x << " " << recSeq3.y << " " << recSeq3.w << " " << recSeq3.h << "\n";
        std::cout << "VECTOR: " << reqVec4.x << " " << reqVec4.y << " " << reqVec4.w << " " << reqVec4.h << "\n";
    }
    {
        vec.pop_back();
        std::cout << "\nPOP BACK VECTOR AGAIN:\n";
        Rect& recSeq3 = seq[seq.size() - 1];
        Rect& reqVec4 = vec[vec.size() - 1];
        std::cout << "SEQUENCE: " << recSeq3.x << " " << recSeq3.y << " " << recSeq3.w << " " << recSeq3.h << "\n";
        std::cout << "VECTOR: " << reqVec4.x << " " << reqVec4.y << " " << reqVec4.w << " " << reqVec4.h << "\n";
    }
    {
        seq.pop_back();
        std::cout << "\nPOP BACK SEQUENCE AGAIN:\n";
        Rect& recSeq3 = seq[seq.size() - 1];
        Rect& reqVec4 = vec[vec.size() - 1];
        std::cout << "SEQUENCE: " << recSeq3.x << " " << recSeq3.y << " " << recSeq3.w << " " << recSeq3.h << "\n";
        std::cout << "VECTOR: " << reqVec4.x << " " << reqVec4.y << " " << reqVec4.w << " " << reqVec4.h << "\n";
    }

}

void poopOut100(std::vector<Rect>& vec, Sequence<Rect>& seq) {
    for (int i = 0; i < 101; ++i) {
        vec.push_back({i,i,i,i});
        seq.add({ i,i,i,i });
    }
}
void poopOut25(std::vector<Rect>& vec, Sequence<Rect>& seq) {
    for (int i = 0; i < 26; ++i) {
        vec.push_back({ i,i,i,i });
        seq.add({ i,i,i,i });
    }
}
void round4Tests_sequence_erase() {
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    {
        std::cout << "\nTEST ERASE 5th ELEMENT:\n";
        Sequence<Rect> seq;
        std::vector<Rect>vec;
        poopOut100(vec, seq);

        auto seqIt = seq.begin() + 4;
        auto vecIt = vec.begin() + 4;

        std::cout << "BEFORE ERASE:\tseq: " << seqIt->x << "\tvec: " << vecIt->x;
        std::cout << "\nBEFORE ERASE LIST:\n";
        std::cout << "seq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';

        for (int i = 0; i < 10; ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }

        auto seqit2 = seq.erase(seqIt);
        auto vecit2 = vec.erase(vecIt);

        std::cout << "AFTER ERASE:\tseq: " << seqit2->x << "\tvec: " << vecit2->x;
        std::cout << "\nAFTER ERASE LIST:\n";

        for (int i = 0; i < 10; ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }

        std::cout << "seq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
    }

    _CrtDumpMemoryLeaks();

    /*
    {
        // DISCOVERING BULLSHIT UNDER THE HOOD...
        std::cout << "\nTEST ERASE 27th ELEMENT WITH CONST ITERATORS:\n";
        Sequence<Rect> seq;
        std::vector<Rect>vec;
        poopOut100(vec, seq);

        const Sequence<Rect> seq2 = seq;
        auto seq2It = seq2.begin();
        // seq2It->x = 7;//can't modify as expected
        auto const seqIt = seq.begin() + 26;
        const auto vecIt = vec.begin() + 26;
        seqIt->x = 7;
        vecIt->x = 7;
        //STOPPED AS IT WAS MEANINGLESS TO GO FURTHER
    }
    _CrtDumpMemoryLeaks();
    */
    

    
    {
        std::cout << "\nTEST ERASE 27th ELEMENT WITH CONST ITERATORS:\n";
        Sequence<Rect> seq;
        std::vector<Rect>vec;
        poopOut100(vec, seq);

        auto seqIt = seq.cbegin() + 26;
        auto vecIt = vec.cbegin() + 26;

        std::cout << "BEFORE ERASE:\tseq: " << seqIt->x << "\tvec: " << vecIt->x;
        std::cout << "\nseq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
        std::cout << "\nBEFORE ERASE LIST:\n";

        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }

        auto seqit2 = seq.erase(seqIt);  //also iterators to const
        auto vecit2 = vec.erase(vecIt);  //also iterators to const


        std::cout << "AFTER ERASE:\tseq: " << seqit2->x << "\tvec: " << vecit2->x;
        std::cout << "\nseq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
        std::cout << "\nAFTER ERASE LIST:\n";

        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }
    }
    
    _CrtDumpMemoryLeaks();
}

void round5Tests_sequence_erase_range() {
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    {
        std::cout << "\nTEST ERASE RANGE FIRST 10:\n";
        Sequence<Rect> seq;
        std::vector<Rect>vec;
        poopOut25(vec, seq);

        std::cout << "\nseq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
        std::cout << "\nBEFORE ERASE LIST:\n";

        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }

        auto seqIt = seq.erase(seq.begin(), seq.begin() + 9);
        auto vecIt = vec.erase(vec.begin(), vec.begin() + 9);

        std::cout << "\nAFTER ERASE:\n";
        std::cout << "seq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
        std::cout << "seq it: " << seqIt->x << "\tvec it:" << vecIt->x << '\n';

        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }

    }

    _CrtDumpMemoryLeaks();


    {
        std::cout << "\n\n\nTEST ERASE RANGE MIDDLE 10:\n";
        Sequence<Rect> seq;
        std::vector<Rect>vec;
        poopOut25(vec, seq);
        std::cout << "\nseq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
        std::cout << "\nBEFORE ERASE LIST:\n";

        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }

        auto seqIt = seq.erase(seq.begin()+9, seq.begin() + 19);
        auto vecIt = vec.erase(vec.begin()+9, vec.begin() + 19);

        std::cout << "\nAFTER ERASE:\n";
        std::cout << "seq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
        std::cout << "seq it: " << seqIt->x << "\tvec it:" << vecIt->x << '\n';

        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }
    }

    _CrtDumpMemoryLeaks();

    {
        std::cout << "\n\n\nTEST ERASE RANGE LAST 10:\n";
        Sequence<Rect> seq;
        std::vector<Rect>vec;
        poopOut25(vec, seq);
        std::cout << "\nseq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';
        std::cout << "\nBEFORE ERASE LIST:\n";
        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }
        auto seqIt = seq.erase(seq.end() - 9, seq.end());
        auto vecIt = vec.erase(vec.end() - 9, vec.end());

        std::cout << "\nAFTER ERASE:\n";
        std::cout << "seq size: " << seq.size() << "\tvec size:" << vec.size() << '\n';

        if (seqIt == seq.end()) {
            std::cout << "\nseq iterator got end iterator\n";
        }
        if (vecIt == vec.end()) {
            std::cout << "\nvec iterator got end iterator\n";
        }
        seqIt -= 1;
        vecIt -= 1;
        std::cout << "seq it -1: " << seqIt->x << "\tvec it -1:" << vecIt->x << '\n';

        for (int i = 0; i < seq.size(); ++i) {
            std::cout << "SEQ: " << seq[i].x << "\tVEC: " << vec[i].x << '\n';
        }
       
    }
    _CrtDumpMemoryLeaks();
}

int main() {
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    round2Tests_testing_iterator_operators();
    round3Tests_sequence_adding_erasing();
    round4Tests_sequence_erase();
    round5Tests_sequence_erase_range();

    {
        Sequence<int> is;
        is.add(1);
        is.add(2);
        is.add(3);
        is.add(4);
    
        auto it = is.begin();
        auto it2 = is.begin()+2;
        auto itc = is.cbegin() + 1;
        it2 = it;
        std::cout << "it: " << *it << '\n';
        std::cout << "itc:" << *itc << '\n';
        std::cout << '\n';
        std::cout << "it == itc: " << (itc == it) << '\n';
        itc = it;
        std::cout << '\n';
        std::cout << "it == itc: " << (it == itc) << '\n';
    }
    std::cout << "\nVECTOR\n";
    {
        std::vector<int> is = { 1,2,3,4 };
    
        auto it = is.begin();
        auto itc = is.cbegin() + 1;
    
        std::cout << "it: " << *it << '\n';
        std::cout << "itc:" << *itc << '\n';
        std::cout << '\n';
        std::cout << "it == itc: " << (it == itc) << '\n';
        itc = it;
        std::cout << '\n';
        std::cout << "it == itc: " << (it == itc) << '\n';
    }
    _CrtDumpMemoryLeaks();

    return 0;
}

