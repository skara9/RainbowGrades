#include <cmath>
#include <cassert>

#include "table.h"
#include "constants_and_globals.h"

bool GLOBAL_instructor_output = false;

bool global_details = false;

// XXX: For now not sanitizing \ since RFC4180 only specifies double quote as an escape
//      i.e. """ is a field with one double quote as the value.
//      In practice many other escape techniques are used, but we'll see how far this gets us
//      and we can always add the \ into the sanitization list if users want it.
//      Also ' is not part of RFC4180 so not stripping those for now either. Probably going
//      to result in some strange behavior in some spreadsheet apps, need user testing.
//      We could relax the stripping, allow commas, and ensure all output uses double quotes
//      to wrap all fields. However, this would make using a text editor to read/edit the CSV painful.
bool CSVSanitizeHelper(const char c){
    return c == '"' || c == ',' || c == '\n' || c == '\r';
}

std::string CSVSanitizeString(const std::string& s){
    std::string ret(s);
    ret.erase(std::remove_if(ret.begin(), ret.end(), CSVSanitizeHelper), ret.end());
    return ret;
}

TableCell::TableCell(const std::string& c, const std::string& d, const std::string& n, int ldu,
                     CELL_CONTENTS_STATUS v, const std::string& a, int s, int r) { 
  assert (c.size() == 6);
  color=c; 
  data=d; 
  note=n; 
  late_days_used=ldu,
  visible=v;
  align=a;
  span=s; 
  rotate=r;
}

TableCell::TableCell(const std::string& c, int d, const std::string& n, int ldu,
                     CELL_CONTENTS_STATUS v, const std::string& a, int s, int r) { 
  assert (c.size() == 6);
  color=c; 
  data=std::to_string(d); 
  note=n; 
  late_days_used=ldu,
  visible=v;
  align=a;
  span=s; 
  rotate=r;
}

TableCell::TableCell(const std::string& c, float d, int precision, const std::string& n, int ldu,
                     CELL_CONTENTS_STATUS v, const std::string& a, int s, int /*r*/) {
  assert (c.size() == 6);
  assert (precision >= 0);
  color=c; 
  if (fabs(d) > 0.0001) {
    std::stringstream ss;
    ss << std::setprecision(precision) << std::fixed << d;
    data=ss.str(); span=s; 
  } else {
    data = "";
  }
  note=n;
  late_days_used=ldu,
  visible=v;
  align=a;
  span=s; 
  rotate = 0;
}


TableCell::TableCell(float d, const std::string& c, int precision, const std::string& n, int ldu,
                     CELL_CONTENTS_STATUS v,const std::string& e,bool ai, const std::string& a, int s, int /*r*/) {
  assert (c.size() == 6);
  assert (precision >= 0);
  color=c;
  if (fabs(d) > 0.0001) {
    std::stringstream ss;
    ss << std::setprecision(precision) << std::fixed << d;
    data=ss.str(); span=s;
  } else {
    data = "";
  }
  note=n;
  late_days_used=ldu,
  visible=v;
  align=a;
  span=s;
  rotate = 0;
  academic_integrity = ai;
  event = e;
  if (event == "Bad"){
    bad_status = true;
    override = inquiry = false;
  } else if ( event == "Overridden"){
    override = true;
    bad_status = inquiry = false;
  } else if (event == "Open"){
    inquiry = true;
    bad_status = override = false;
  } else {
   inquiry = bad_status = override = false; 
  }
    
}



std::ostream& operator<<(std::ostream &ostr, const TableCell &c) {
  assert (c.color.size() == 6);
    
    std::string outline = "";
    std::string mark = "";
    if (c.academic_integrity){
        outline = "outline:4px solid #0a0a0a; outline-offset: -4px;";
        mark = "@";
    } else if (c.override){
        outline = "outline:4px solid #fcca03; outline-offset: -4px;";
    } else if (c.inquiry){
        outline = "outline:4px dashed #1cfc03; outline-offset: -4px;";
    } else if (c.bad_status){
        outline = "outline:4px solid #fc0303; outline-offset: -4px;";
    }
    
  //  ostr << "<td bgcolor=\"" << c.color << "\" align=\"" << c.align << "\">";
  ostr << "<td style=\"border:1px solid #aaaaaa; background-color:#" << c.color << "; " << outline << " \" align=\"" << c.align << "\">";
    
  if (0) { //rotate == 90) {
    ostr << "<div style=\"position:relative\"><p class=\"rotate\">";
  }
  ostr << "<font size=-1>";
  std::string mynote = c.getNote();

  if (c.academic_integrity)
  {
    ostr << mark;
  }
  else if ((c.data == "" && mynote=="")
      || c.visible==CELL_CONTENTS_HIDDEN
      || (c.visible==CELL_CONTENTS_VISIBLE_INSTRUCTOR && GLOBAL_instructor_output == false) 
      || (c.visible==CELL_CONTENTS_VISIBLE_STUDENT    && GLOBAL_instructor_output == true)) {
    ostr << "<div></div>";
  } else {
    ostr << c.data; 
    if (c.late_days_used > 0) {
      if (c.late_days_used > 3) { ostr << " (" << std::to_string(c.late_days_used) << "*)"; }
      else { ostr << " " << std::string(c.late_days_used,'*'); }
    }
      
    
    if (mynote.length() > 0 &&
        mynote != " " &&
        (global_details 
         /*
        || 
        c.visible==CELL_CONTENTS_HIDDEN
         */
         )
        ) {
      ostr << "<br><em>" << mynote << "</em>";
    }
  }
  ostr << "</font>";
  if (0) { //rotate == 90) {
    ostr << "</p></div>";
  }
  ostr << "</td>";
  return ostr;
}

std::string TableCell::make_cell_string(bool csv_mode) const{
    std::string ret;
    std::string mynote = this->getNote();
    if ((this->data == "" && mynote=="")
        || this->visible==CELL_CONTENTS_HIDDEN
        || (this->visible==CELL_CONTENTS_VISIBLE_INSTRUCTOR && GLOBAL_instructor_output == false)
        || (this->visible==CELL_CONTENTS_VISIBLE_STUDENT    && GLOBAL_instructor_output == true)) {
        //ret += "\n"; //Empty cell
    } else {
        ret += this->data;
        if (this->late_days_used > 0 && csv_mode == false) {
            if (this->late_days_used > 3) { ret += " (" + std::to_string(this->late_days_used) + "*)"; }
            else { ret += " " + std::string(this->late_days_used,'*'); }
        }
        if (mynote.length() > 0 &&
            mynote != " " &&
            global_details) {
            ret += mynote;
            std::cerr << "Printing note: " << mynote << std::endl;
        }
    }
    return CSVSanitizeString(ret);
}



void Table::output(std::ostream& ostr,
                   std::vector<int> which_students,
                   std::vector<int> which_data,
                   bool csv_mode,
                   bool transpose,
                   bool show_details,
                   std::string last_update) const {

  global_details = show_details;
  //global_details = true;

  if(!csv_mode) {
      ostr << "<style>\n";
      ostr << ".rotate {\n";
      ostr << "             filter:  progid:DXImageTransform.Microsoft.BasicImage(rotation=0.083);  /* IE6,IE7 */\n";
      ostr << "         -ms-filter: \"progid:DXImageTransform.Microsoft.BasicImage(rotation=0.083)\"; /* IE8 */\n";
      ostr << "     -moz-transform: rotate(-90.0deg);  /* FF3.5+ */\n";
      ostr << "      -ms-transform: rotate(-90.0deg);  /* IE9+ */\n";
      ostr << "       -o-transform: rotate(-90.0deg);  /* Opera 10.5 */\n";
      ostr << "  -webkit-transform: rotate(-90.0deg);  /* Safari 3.1+, Chrome */\n";
      ostr << "          transform: rotate(-90.0deg);  /* Standard */\n";
      ostr << " display:block;\n";
      ostr << " position:absolute;\n";
      ostr << " right:-50%;\n";
      ostr << "}\n";
      ostr << "</style>\n";
  }


  // -------------------------------------------------------------------------------
  // PRINT INSTRUCTOR SUPPLIED MESSAGES
  if(!csv_mode) {
      for (unsigned int i = 0; i < MESSAGES.size(); i++) {
          ostr << "" << MESSAGES[i] << "<br>\n";
      }
      if (last_update != "") {
          ostr << "<em>Information last updated: " << last_update << "</em><br>\n";
      }
      ostr << "&nbsp;<br>\n";
      ostr << "<table style=\"border:1px solid #aaaaaa; background-color:#aaaaaa;\">\n";
  }
  
  if (transpose) {
    for (std::vector<int>::iterator c = which_data.begin(); c != which_data.end(); c++) {
      ostr << "<tr>\n";
      for (std::vector<int>::iterator r = which_students.begin(); r != which_students.end(); r++) {
        ostr << cells[*r][*c] << "\n";
      }
      ostr << "</tr>\n";
    }
  } else {
      //CSV only runs in transpose = false
      if(!csv_mode){
          for (std::vector<int>::iterator r = which_students.begin(); r != which_students.end(); r++) {
              ostr << "<tr>\n";
              for (std::vector<int>::iterator c = which_data.begin(); c != which_data.end(); c++) {
                  ostr << cells[*r][*c] << "\n";
              }
              ostr << "</tr>\n";
          }
      }
      else{
          for (std::vector<int>::iterator r = which_students.begin(); r != which_students.end(); r++) {
              bool first_cell = true;
              for (std::vector<int>::iterator c = which_data.begin(); c != which_data.end(); c++) {
                  if(first_cell){
                      first_cell = false;
                  }
                  else{
                      ostr << ",";
                  }
                  ostr << cells[*r][*c].make_cell_string(csv_mode);
              }
              ostr << "\n";
          }
      }
  } 

  if(!csv_mode) {
      ostr << "</table>" << std::endl;
  }
}
