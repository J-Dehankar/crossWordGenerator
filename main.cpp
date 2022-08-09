#include<iostream>
#include<vector>
#include<assert.h>
#include<fstream>
#include<unordered_map>
#include<unordered_set>

using namespace std;


// ---------------------------------------------
typedef unordered_set<string> StringSet;

bool existInSet(const StringSet& set, const string& s) {
    auto it = set.find(s);
    return it != set.end();
}

void addToSet(StringSet& set, const string& s) {
    assert(!existInSet(set, s));
    set.insert(s);
}

// ---------------------------------------------
string toUpper(string s) {
    string s2;
    for(char c : s) {
        s2.push_back(toupper(c));
    }
    return s2;
}

// ---------------------------------------------
struct Point {
    Point() {}
    Point(int r, int c) : row(r), col(c) {}

    friend ostream& operator << (ostream& os, const Point& p);

    int row=0, col=0;
};
ostream& operator << (ostream& os, const Point& p) {
    os << "(" << p.row << ", " << p.col << ")";
    return os;
}

// ---------------------------------------------
struct Span
{
    Span(Point p, int l, bool v) : point(p), len(l), vert(v) {}
    friend ostream& operator << (ostream& os, const Span& s);
    
    Point getPoint(int i) const {
        assert(i >= 0 && i < len);
        if(vert) {
            return Point(point.row + i, point.col);
        }
        else {
            return Point(point.row, point.col + i);
        }
    }
    
    Point point;
    int len;
    bool vert;
};
ostream& operator << (ostream& os, const Span& s) {
    os << " [" << s.point << ", len=" << s.len << ", vert=" << s.vert << "] ";
    return os;
}
typedef vector<Span> Spans;


// ---------------------------------------------
struct Word {
    Word(){}
    Word(string s) : word(s){}
    int len() const {
        return word.length();
    }
    string word;
};
typedef vector<Word*> Words; 
typedef unordered_map<string, Words> WordMap;


// ---------------------------------------------
class library {
    
    public:
        // constructor
        library() {}

        //destructor
        ~library() {
            for(Word* w : words_) {
                delete w;
            }
        }
    
        const Words* findWord(const string& s) const {
            auto it = word_map_.find(s);
            if(it != word_map_.end()) {
                return &it->second;
            }
            return NULL;
        }
        
        bool isWord(string s) {            
            auto it = word_map_.find(s);
            if(it == word_map_.end()) return false;
            else return true;
        }

        void computeStats() {
            assert(counts_.empty());
            counts_.resize(18);
            for(Word* w : words_) {
                int len = w->word.length();
                if(len < 18)
                    counts_[len]++;
            }
        }

        void printStats() const {
            for(int i=1; i<18; i++) {
                cout << "[ " << i << " ]" << "\t" << counts_[i] << endl;
            }
        }

        string getWord(int i) const {
            assert(i >= 0 && i < words_.size());
            return words_[i]->word;
        }

        void createPatternHash(Word* w, int max_size) {
            int len = w->len();
            if(len > max_size) return;
            int num_patterns = 1 << len;
            for(int i=0; i < num_patterns; i++) {
                string temp = w->word;
                for(int j=0; j<len; j++) {
                    if((i >> j) & 1) {
                        temp[j] = '-';
                    }
                }
                // cout << temp << endl;
                word_map_[temp].push_back(w);
            }
            
        }

        void readFromFile(string filename, int max_size) {
            ifstream f;
            f.open(filename);
            while(f.is_open() && !f.eof()) {
                string line;
                getline(f, line);
                if(!line.empty() & line.length() <= max_size) {
                    line = toUpper(line);
                    // word_map_[line] = Word(line);
                    Word* w = new Word(line);
                    words_.push_back(w);
                    createPatternHash(w, max_size);
                }
            }
            cout << "Read " << words_.size() << " words from '" << filename << "' file" << endl;
        }

        void printShelves() {
            for(int i=0; i<word_map_.bucket_count(); i++) {
                cout << " " << i << " " << word_map_.bucket_size(i) << endl;
            }
        }

    private:
        Words words_; // master vector of words
        WordMap word_map_; //patternhash e.g. { key = "D--" | value = ["DAD", "DID", "DUD", ...] }
        vector<int> counts_;
};
library lib;

// ---------------------------------------------
struct Attr
{
    bool is_empty() const {return !has_letters && has_blanks;}
    bool is_partial() const {return has_letters && has_blanks;}
    bool is_full() const {return has_letters && !has_blanks;}
    bool has_letters = false;
    bool has_blanks = false;
};


// ---------------------------------------------
struct Grid
{
   
    Grid() {
        name = "UNTITLED";
    }

    Grid(string n) {
        name = n;
    }

    int max_size() const {
        return max(rows(), cols());
    }

    int rows() const {
        return lines.size();
    }
    

    int cols() const {
        if(!lines.empty()) {
            return lines[0].size();            
        }
        return 0;
    }

    void write_box(const Point& p, char c) {
        assert(in_bounds(p));
        lines[p.row][p.col] = c;
    }

    char box(const Point& p) const {
        assert(in_bounds(p));
        return lines[p.row][p.col];
    }

    bool is_block(const Point& p) const {
        return box(p) == '.';
    }

    bool is_blank(const Point& p) const {
        return box(p) == '-';
    }

    bool is_letter(const Point& p) const {
        return (box(p) >= 'A' && box(p) <= 'Z');
    }

    bool in_bounds(const Point& p) const {
        return (p.row >= 0 && p.row < rows() && p.col >= 0 && p.col < cols());
    }

    string getString(const Span& s, Attr& attr) const {
        Point p;
        int len = s.len;
        string temp;
        temp.resize(len);
        for(int i=0; i<len; i++) {
            p = s.getPoint(i);
            char c = box(p);
            temp[i] = c;
            if(c == '-') {
                attr.has_blanks = true;
            }
            else if(c >= 'A' && c <= 'Z') {
                attr.has_letters = true;
            }
        }
        return temp;
    }

    void writeString(const Span& s, const string& t) {
        int len = s.len;
        assert(t.length() == len);
        for(int i=0; i<len; i++) {
            Point p = s.getPoint(i);
            write_box(p, t[i]);
        }
    }

    bool next(Point& p, bool vert) {
        if(vert) {
            p.row++;
            if(p.row >= rows()) {
                p.row = 0;
                p.col++;
            }
        }
        else {
            p.col++;
            if(p.col >= cols()) {
                p.col = 0;
                p.row++;
            }
        }

        return in_bounds(p);
    }

    bool nextStopAtWrap(Point& p, bool vert) {
        if(vert) {
            p.row++;
            if(p.row >= rows()) {
                p.row = 0;
                p.col++;
                return false;
            }
        }
        else {
            p.col++;
            if(p.col >= cols()) {
                p.col = 0;
                p.row++;
                return false;
            }
        }

        return true;
    }

    void fillSpans(bool vert) {

        Point p;
        while(in_bounds(p)) {

            while (in_bounds(p) && is_block(p)) {next(p, vert);}

            if (!in_bounds(p)) return;

            Point startp = p;
            int len = 0;
            bool flag;
            //cout << "START " << p << endl;
            do {
                flag = nextStopAtWrap(p, vert);
                len++;
            } while (flag && !is_block(p));

            //cout << "END " << p << endl << "LEN " << len << endl;
            spans.push_back(Span(startp, len, vert));
        }
    }

    void fillSpans() {
        assert(spans.empty());
        // cout << "Horizontal" << endl;
        fillSpans(false);
        // cout << "Vertical" << endl;
        fillSpans(true);
    }

    void loadfromFile(string filename) {
        ifstream f;
        f.open(filename);
        
        while(f.is_open() && !f.eof()) {
            string line;
            getline(f, line);
            if(!line.empty() && line[0] != '#') {
                // cout << line << "\t" << line.length() << "\n";
                lines.push_back(line);
            }
        }
    }

    void check() const {
        for(string line : lines) {
            assert(line.size() == cols());
        }

    }

    void printGrid() const {
        cout << "Grid Name : " << name << " (Rows = " << rows() << ", Cols = " << cols()<< ") " << "Max Size "<< max_size() << endl;
    
        for(string line : lines) {
            cout << line << endl;
        }       

    }

    void printSpan() const {
        for(const Span& s: spans) {
            Attr attr;
            cout << s << " " << getString(s, attr) << endl;
        }
    }

    string name;
    vector<string> lines;
    Spans spans;

};


// ---------------------------------------------
struct Slot
{
    Slot(Span s, const string& p) : span(s), pattern(p) {}
    friend ostream& operator << (ostream& os, const Slot& s);

    Span span;
    string pattern;
    
};
ostream& operator << (ostream& os, const Slot& s) {
    os << s.span << " '" << s.pattern << "' ";
    return os;
}
typedef vector<Slot> Slots;


// ---------------------------------------------
class Solver {
    public:

        void solve(const Grid& grid) {
            cout << "Solving Grid" << endl;
            grid.printGrid();
            loop(grid);
        }

    private:
        void loop(Grid grid) const {

            Slots full_slots;
            Slots partial_slots;
            Slots empty_slots;
            for(const Span& s : grid.spans) {
                Attr attr;
                string temp = grid.getString(s, attr);
                if(attr.is_empty()) {
                    empty_slots.push_back(Slot(s, temp));
                }
                else if(attr.is_partial()) {
                    partial_slots.push_back(Slot(s, temp));
                }
                else if(attr.is_full()) {
                    full_slots.push_back(Slot(s, temp));
                }
            }
            int num_empty = empty_slots.size();
            int num_partial = partial_slots.size();
            int num_full = full_slots.size();

            for(const Slot& s : full_slots) {
                // cout << "CHECKING " << s.pattern << " if it is a word" << endl;
                if(!lib.isWord(s.pattern)) {
                    // cout << "--> NOT, Abort!" << endl;
                    return;
                }
            }

            //need to check all the words are unique! No duplicates allowed!
            StringSet set;
            for(const Slot& s: full_slots) {
                if(existInSet(set, s.pattern)) {
                    return;
                }
                addToSet(set, s.pattern);
            }

            if(num_empty == 0 && num_partial == 0) {
                cout << "SOLUTION !!";
                grid.printGrid();
                return;
            }
            
            assert(num_partial > 0);
            commitSlot(grid, partial_slots[0]);
        }

        void commitSlot(Grid& grid, const Slot& slot) const {
            // cout << "COMMIT SLOT " << slot << endl;
            // cout << "Possible choices of the words are : " << endl;

            const Words* words = lib.findWord(slot.pattern);
            if(words) {
                for(const Word* w : *words) {
                    // cout << "Commiting '" << w->word << "'\n";
                    grid.writeString(slot.span, w->word);
                    // grid.printGrid();
                    loop(grid);
                }
            }
            else {
                // cout << "NO matches for pattern" << endl;
            }
        }
};


// ---------------------------------------------
int main() {

    Grid grid("New Grid");
    grid.loadfromFile("test");
    grid.printGrid();
    grid.fillSpans();

    lib.readFromFile("top_10000.txt", grid.max_size());
    
    Solver solver;
    solver.solve(grid);

}