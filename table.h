#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

extern bool GLOBAL_instructor_output;

enum CELL_CONTENTS_STATUS { CELL_CONTENTS_VISIBLE, CELL_CONTENTS_HIDDEN, CELL_CONTENTS_VISIBLE_STUDENT, CELL_CONTENTS_VISIBLE_INSTRUCTOR, CELL_CONTENTS_NO_DETAILS };

//Helper function for sanitization
bool CSVSanitizeHelper(const char c);
std::string CSVSanitizeString(const std::string& s);

class TableCell {
public:

  // CONSTRUCTORS
  TableCell(const std::string& c="ffcccc", const std::string& d="", const std::string& n="", int ldu=0,
            CELL_CONTENTS_STATUS v=CELL_CONTENTS_VISIBLE, const std::string& a="left" , int s=1, int r=0);
  TableCell(const std::string& c         , int                d   , const std::string& n="", int ldu=0,
            CELL_CONTENTS_STATUS v=CELL_CONTENTS_VISIBLE, const std::string& a="left" , int s=1, int r=0);
  TableCell(const std::string& c         , float              d   , int precision, const std::string& n="", int ldu=0,
            CELL_CONTENTS_STATUS v=CELL_CONTENTS_VISIBLE, const std::string& a="right", int s=1, int r=0);
  TableCell(float              d   ,const std::string& c         , int precision, const std::string& n="", int ldu=0,
            CELL_CONTENTS_STATUS v=CELL_CONTENTS_VISIBLE, const std::string& e="", bool ai = false, const std::string& a="right", int s=1, int r=0);
  
  std::string make_cell_string(bool csv_mode) const;
  std::string color;
  std::string data;
  std::string event;
  int late_days_used;
  bool academic_integrity = false;
  bool inquiry = false;
  bool bad_status = false;
  bool override = false;
  std::string align;
  enum CELL_CONTENTS_STATUS visible;
  int span;
  int rotate;
  friend std::ostream& operator<<(std::ostream &ostr, const TableCell &c);
  const std::string& getNote() const { return note; }
private:
  std::string note;
};



class Table {

public:

  Table() {
    cells = std::vector<std::vector<TableCell>>(1,std::vector<TableCell>(1));
  }

  void set(int r, int c, TableCell cell) {
    while(r >= numRows()) {
      cells.push_back(std::vector<TableCell>(numCols()));
    }
    while (c >= numCols()) {
      for (int x = 0; x < numRows(); x++) {
        cells[x].push_back(TableCell());
      }
    }
    cells[r][c] = cell;
  }

  void output(std::ostream& ostr,
              std::vector<int> which_students,
              std::vector<int> which_data,
              bool csv_mode=false,
              bool transpose=false,
              bool show_details=false,
              std::string last_update="") const;
  

  int numRows() const { return cells.size(); }
  int numCols() const { return cells[0].size(); }

private:
  std::vector<std::vector<TableCell>> cells;
};
