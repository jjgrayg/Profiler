/*
 *  ASTree.cpp
 *  Abstract Syntax Tree
 *
 *  Created by Jonathan Maletic
 *  Copyright 2021 Kent State University. All rights reserved.
 *  Spring 2021
 *  Modified by: Jarod Graygo
 *
 */

#include "ASTree.hpp"


/////////////////////////////////////////////////////////////////////
// Copy constructor for srcML
//
srcML::srcML(const srcML& actual) {
    header = actual.header;
    if (actual.tree)
        tree   = new AST(*(actual.tree));
    else
        tree = 0;
}

/////////////////////////////////////////////////////////////////////
// Constant time swap for srcML
//
void srcML::swap(srcML& b) {
    std::string t_header = header;
    header = b.header;
    b.header = t_header;
    
    AST *temp = tree;
    tree = b.tree;
    b.tree = temp;
}

/////////////////////////////////////////////////////////////////////
// Assignment for srcML
//
srcML& srcML::operator=(srcML rhs) {
    swap(rhs);
    return *this;
}

/////////////////////////////////////////////////////////////////////
// Reads in and constructs a srcML object.
//
std::istream& operator>>(std::istream& in, srcML& src){
    char ch;
    if (!in.eof()) in >> ch;
    src.header = readUntil(in, '>');
    if (!in.eof()) in >> ch;
    if (src.tree) delete src.tree;
    src.tree = new AST(category, readUntil(in, '>'));
    src.tree->read(in);
    return in;
}


/////////////////////////////////////////////////////////////////////
// Prints out a srcML object
//
std::ostream& operator<<(std::ostream& out, const srcML& src){
    if (src.tree) src.tree->print(out);
    return out;
}

/////////////////////////////////////////////////////////////////////
//  Adds in the includes and profile variables
//
void srcML::mainHeader(const std::vector<std::string>& profileName) {
    tree->mainHeader(profileName);
}

/////////////////////////////////////////////////////////////////////
//  Adds in the includes and profile variables
//
void srcML::fileHeader(const std::string& profileName) {
    tree->fileHeader(profileName);
}


/////////////////////////////////////////////////////////////////////
// Adds in the report to the main.
//
void srcML::mainReport(const std::vector<std::string>& profileName) {
        tree->mainReport(profileName);
}

/////////////////////////////////////////////////////////////////////
//  Inserts a filename.count() into each function body.
//
void srcML::funcCount(const std::string& profileName) {
    tree->funcCount(profileName);
}

/////////////////////////////////////////////////////////////////////
// Inserts a filename.count() for each statement.
//
void srcML::lineCount(const std::string& profileName) {
    tree->lineCount(profileName);
}

    

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// Constructs a category, token, or whitespace node for the tree.
//
AST::AST(nodes t, const std::string& s) {
    nodeType = t;
    switch (nodeType) {
        case category:
            tag = s;
            break;
        case token:
            text = unEscape(s);
            break;
        case whitespace:
            text = s;
            break;
    }
}


/////////////////////////////////////////////////////////////////////
// Destructor for AST
//
AST::~AST() {

    // Check if the AST has children
    if (child.size() != 0) {

        // If it does iterate over each one
        for (std::list<AST*>::const_iterator i = child.begin(); i != child.end(); ++i) {

            // Recursively call the destructor over each child
            delete *i;
        }
    }
}


/////////////////////////////////////////////////////////////////////
// Copy Constructor for AST
//
AST::AST(const AST& actual) {

    // Check if the AST has children
    if (actual.child.size() != 0) {

        // If it does iterate over each one
        for (std::list<AST*>::const_iterator i = actual.child.begin(); i != actual.child.end(); ++i) {

            // Recursively call the copy ctor
            AST* temp = new AST(*(*i));

            // Add each new AST to the list of children
            child.push_back(temp);
        }
    }

    // Copy over the elements from actual to this
    text = actual.text;
    nodeType = actual.nodeType;
    tag = actual.tag;
    closeTag = actual.closeTag;
}


/////////////////////////////////////////////////////////////////////
// Constant time swap for AST
//
void AST::swap(AST& b) {

    // Swap nodes
    nodes tempNode = b.nodeType;
    b.nodeType = nodeType;
    nodeType = tempNode;

    // Swap tags
    std::string tempTag = b.tag;
    b.tag = tag;
    tag = tempTag;
    tempTag = b.closeTag;
    b.closeTag = closeTag;
    closeTag = tempTag;

    // Swap children
    std::list<AST*> tempList = b.child;
    b.child = child;
    child = tempList;

    // Swap text
    std::string tempText = b.text;
    b.text = text;
    text = tempText;
}

/////////////////////////////////////////////////////////////////////
// Assignment for AST
//
AST& AST::operator=(AST rhs) {
    swap(rhs);
    return *this;
}


/////////////////////////////////////////////////////////////////////
// Returns a pointer to child[i] where (child[i]->tag == tagName)
//
// IMPORTANT for milestone 3
//
AST* AST::getChild(std::string tagName) {
    std::list<AST*>::iterator ptr = child.begin();
    while (((*ptr)->tag != tagName) && (ptr != child.end())) {
         ++ptr;
    }
    return *ptr;
}


/////////////////////////////////////////////////////////////////////
// Returns the full name of a <name> node.
//  There are two types of names.  A simple name (e.g., foo) and a
//   name with a scope (e.g., std::bar).  This returns the correct
//   one from an AST node.
//
// IMPORTANT for milestone 3
//
std::string AST::getName() const {
    std::string result;
    if (child.front()->tag != "name") {
        result = child.front()->text;   //A simple name (e.g., main)
    } else {                            //A complex name (e.g., stack::push).
        result = child.front()->child.front()->text;
        result += "::";
        result += child.back()->child.front()->text;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////
//  Adds in the includes and profile variables in a main file.
//
void AST::mainHeader(const std::vector<std::string>& profileName) {

    std::list<AST*>::iterator ptr = child.begin();
    std::list<AST*>::iterator prevPtr;
    while (ptr != child.end()) {
        if ((*ptr)->tag == "function") {
            break;
        }
        prevPtr = ptr;
        ++ptr;
    }

    /////////////////////////////////////////////////////////////////////
    // Create include directive
    AST* cpp_include = new AST(token, "\n\n// Include header for profiling\n#include \"profile.hpp\"\n");
    child.insert(prevPtr, cpp_include);
    /////////////////////////////////////////////////////////////////////
    // Create profile declaration for each in profileName
    for (unsigned long i = 0; i < profileName.size(); ++i) {
        std::string profName = profileName[i];
        std::string profileDec = "profile " + profName + "(\"";
        unsigned int lastUnderscoreIndex = 0;
        for (unsigned int j = 0; j < profName.size(); ++j) {
            if (profName[j] == '_') lastUnderscoreIndex = j;
        }
        profName[lastUnderscoreIndex] = '.';
        profileDec += profName + "\");\n";
        AST* profNode = new AST(token, profileDec);

        child.insert(prevPtr, profNode);
    }

}

/////////////////////////////////////////////////////////////////////
//  Adds in the includes and profile variables for non-main files
//
void AST::fileHeader(const std::string& profileName) {

    std::list<AST*>::iterator ptr = child.begin();
    std::list<AST*>::iterator prevPtr;
    while (ptr != child.end()) {
        if ((*ptr)->tag == "function") {
            break;
        }
        prevPtr = ptr;
        ++ptr;
    }

    /////////////////////////////////////////////////////////////////////
    // Create include directive
    AST* cpp_include = new AST(token, "\n\n// Include header for profiling\n#include \"profile.hpp\"\n");
    child.insert(prevPtr, cpp_include);
    /////////////////////////////////////////////////////////////////////
    // Create profile declaration for each in profileName
    std::string profName = profileName;
    std::string profileDec = "extern profile " + profName + "(\"";
    int lastUnderscoreIndex = 0;
    for (unsigned int i = 0; i < profName.size(); ++i) {
        if (profName[i] == '_') lastUnderscoreIndex = i;
    }
    profName[lastUnderscoreIndex] = '.';
    profileDec += profName + "\");\n";
    AST* profNode = new AST(token, profileDec);

    child.insert(prevPtr, profNode);

}


/////////////////////////////////////////////////////////////////////
// Adds in the report to the main.
// Assumes only one return at end of main body.
//
void AST::mainReport(const std::vector<std::string>& profileName) {
    
    //NEED TO IMPLEMENT
    
    //Find the main - function with name of "main"
    //Then start from the end() of this function and iterate
    // backwards until you find a return stmt.   You'll want
    // to insert the report statements before this return.

    std::list<AST*>::iterator ptr = child.begin();
    std::list<AST*>::iterator innerFunctionPtr;
    std::list<AST*>::iterator blockPtr;
    AST* ptrToMain;
    AST* ptrToReturn;
    std::string profName;
    std::string outStatement;

    AST* outNode;

    // Finds the function with name "main"
    while (ptr != child.end()) {
        if ((*ptr)->tag == "function") {
            innerFunctionPtr = (*ptr)->child.begin();
            while (innerFunctionPtr != (*ptr)->child.end()) {
                if ((*innerFunctionPtr)->tag == "name") {
                    if ((*((*innerFunctionPtr)->child.begin()))->text == "main") {
                        ptrToMain = *ptr;
                    }
                }
                ++innerFunctionPtr;
            }
        }
        ++ptr;
    }

    // Finds the return call within "main"
    ptr = ptrToMain->child.begin();
    while (ptr != ptrToMain->child.end()) {
        if ((*ptr)->tag == "block") {
            blockPtr = (*ptr)->child.begin();
            while (blockPtr != (*ptr)->child.end()) {
                if ((*blockPtr)->tag == "return") {
                    ptrToReturn = *blockPtr;
                    break;
                }
                ++blockPtr;
            }
        }
        if (ptrToReturn) break;
        ++ptr;
    }

    for (unsigned long i = 0; i < profileName.size(); ++i) {
        profName = profileName[i];
        outStatement = "std::cout << " + profName + " << std::endl;\n\t";

        outNode = new AST(token, outStatement);

        child.insert(blockPtr, outNode);
    }
    
}


/////////////////////////////////////////////////////////////////////
// Adds in a line to count the number of times each function is executed.
//  Assumes no nested functions.
//
void AST::funcCount(const std::string& profileName) {
    
    //NEED TO IMPLEMENT
    
    // for all children
    //     if child is a function, constructor, destructor
    //        Find the function name (use AST::getName())
    //        Find block and insert count as first line in block
    //

    std::list<AST*>::iterator ptr = child.begin();
    std::list<AST*>::iterator nameFinder;
    std::list<AST*>::iterator blockPtr;
    std::string nameOfFunc;
    std::string countStr;

    // Find each function, constructor, and destructor
    while (ptr != child.end()) {
        if ((*ptr)->tag == "function" || (*ptr)->tag == "constructor" || (*ptr)->tag == "destructor") {
            nameFinder = (*ptr)->child.begin();
            while (nameFinder != (*ptr)->child.end()) {
                if ((*nameFinder)->tag == "name") {
                    nameOfFunc = (*nameFinder)->getName();
                    break;
                }
                ++nameFinder;
            }

            blockPtr = (*ptr)->child.begin();
            while (blockPtr != (*ptr)->child.end()) {
                if ((*blockPtr)->tag == "block") {
                    blockPtr = (*blockPtr)->child.begin();
                    ++blockPtr;
                    break;
                }
                ++blockPtr;
            }
            
            countStr = " " + profileName + ".count(__LINE__, \"" + nameOfFunc + "\");";
            AST* func = new AST(token, countStr);

            child.insert(blockPtr, func);
        }
        ++ptr;
    }

}


/////////////////////////////////////////////////////////////////////
// Adds in a line to count the number of times each statement is executed.
//   No breaks, returns, throw etc.
//   Assumes all construts (for, while, if) have { }.
//
void AST::lineCount(const std::string& profileName) {
    
    //NEED TO IMPLEMENT
    
    // Recursively check for expr_stmt within all blocks
    // The basis is when isStopTag is true.

    std::list<AST*>::iterator ptr = child.begin();
    std::list<AST*>::iterator blockPtr;
    std::list<AST*>::iterator exprFinder;
    std::string nameOfFunc;
    std::string countStr;

    std::vector<AST*> ptrs = deepScan("expr_stmt", ptrs);
    for (unsigned long i = 0; i < ptrs.size(); ++i) {
        std::cout << ptrs.size();
        ptrs[i]->print(std::cout);
    }
    
    while (ptr != child.end()) {
        if ((*ptr)->tag == "function" || (*ptr)->tag == "constructor" || (*ptr)->tag == "destructor") {
            std::cout << "I'm in the first if" << std::endl;//
            blockPtr = (*ptr)->child.begin();
            while (blockPtr != (*ptr)->child.end()) {
                std::cout << "I'm in the second while" << std::endl;//
                if ((*blockPtr)->tag == "block") {
                    std::cout << "I'm in the second if" << std::endl;//
                    break;
                }
                ++blockPtr;
            }
            
            countStr = " " + profileName + ".count(__LINE__);";
            AST* func = new AST(token, countStr);

            exprFinder = (*blockPtr)->child.begin();
            while (exprFinder != (*blockPtr)->child.end()) {
                std::cout << "I'm in the third while" << std::endl;//
                if ((*exprFinder)->tag == "expr_stmt") {
                    std::cout << "I'm in the third if" << std::endl;//
                    child.insert(++exprFinder, func);
                }
                ++exprFinder;
            }
        }
        ++ptr;
    }
    
} 

std::vector<AST*>& AST::deepScan(std::string searchTag, std::vector<AST*>& vecToPopulate) {
    std::vector<AST*> temp;
    std::list<AST*>::iterator ptr = child.begin();
    if (child.empty()){
        return vecToPopulate;
    }
    while (ptr != child.end()) {
        if (((*ptr)) && (*ptr)->tag == searchTag) {
            (*ptr)->print(std::cerr);
            vecToPopulate.push_back(*ptr);
            return vecToPopulate;
        }
        temp = (*ptr)->deepScan(searchTag, vecToPopulate);
        for (unsigned long i = 0; i < temp.size(); ++i) {
            vecToPopulate.push_back(temp[i]);
        }
        ++ptr;
    }
    return vecToPopulate;
}


/////////////////////////////////////////////////////////////////////
// Read in and construct AST
// REQUIRES: '>' was previous charater read 
//           && this == new AST(category, "TagName")
//
//
std::istream& AST::read(std::istream& in) {
    AST *subtree;
    std::string temp, Lws, Rws;
    char ch;
    if (!in.eof()) in.get(ch);
    while (!in.eof()) {
        if (ch == '<') {                      //Found a tag
            temp = readUntil(in, '>');
            if (temp[0] == '/') {
                closeTag = temp;
                break;                        //Found close tag, stop recursion
            }
            subtree = new AST(category, temp);               //New subtree
            subtree->read(in);                               //Read it in
            in.get(ch);
            child.push_back(subtree);                        //Add it to child
        } else {                                             //Found a token
            temp = std::string(1, ch) + readUntil(in, '<');  //Read it in.
            std::vector<std::string> tokenList = tokenize(temp);
            for (std::vector<std::string>::const_iterator i=tokenList.begin();
                 i != tokenList.end();
                 ++i) {
                if (isspace((*i)[0])) {
                    subtree = new AST(whitespace, *i);
                } else {
                    subtree = new AST(token, *i);
                }
                child.push_back(subtree);
            }
            ch = '<';
        }
    }
    return in;
}


/////////////////////////////////////////////////////////////////////
// Print an AST
// Preorder traversal that prints out leaf nodes only (tokens & whitesapce)
//
std::ostream& AST::print(std::ostream& out) const {
    for (std::list<AST*>::const_iterator i = child.begin(); i != child.end(); ++i) {
        if ((*i)->nodeType != category)
            out << (*i)->text;   //Token or whitespace node
        else
            (*i)->print(out);    //Category node
    }
    return out;
}

    

/////////////////////////////////////////////////////////////////////
// Utilities
//


/////////////////////////////////////////////////////////////////////
// This returns true if a syntactic category is encountered that
//  will not be profiled.
//
// This is IMPORTANT for milestone 3
//
bool isStopTag(std::string tag) {
    if (tag == "decl_stmt"            ) return true;
    if (tag == "argument_list"        ) return true;
    if (tag == "init"                 ) return true;
    if (tag == "condition"            ) return true;
    if (tag == "cpp:include"          ) return true;
    if (tag == "macro"                ) return true;
    if (tag == "comment type\"block\"") return true;
    if (tag == "comment type\"line\"" ) return true;
    return false;
}


/////////////////////////////////////////////////////////////////////
// Reads until a key is encountered.  Does not include ch.
// REQUIRES: in.open()
// ENSURES: RetVal[i] != key for all i.
//
std::string readUntil(std::istream& in, char key) {
    std::string result;
    char ch;
    in.get(ch);
    while (!in.eof() && (ch != key)) {
        result += ch;
        in.get(ch);
    }
    return result;
}


/////////////////////////////////////////////////////////////////////
// Converts escaped XML charaters back to charater form
// REQUIRES: s == "&lt;"
// ENSURES:  RetVal == "<"
//
std::string unEscape(std::string s) {
    std::size_t pos = 0;
    while ((pos = s.find("&gt;"))  != s.npos) { s.replace(pos, 4, ">");}
    while ((pos = s.find("&lt;"))  != s.npos) { s.replace(pos, 4, "<");}
    while ((pos = s.find("&amp;")) != s.npos) { s.replace(pos, 5, "&");}
    return s;
}


/////////////////////////////////////////////////////////////////////
// Given: s == "   a + c  "
// RetVal == {"   ", "a", " ", "+", "c", " "}  
//
std::vector<std::string> tokenize(const std::string& s) {
    std::vector<std::string> result;
    std::string temp = "";
    unsigned i = 0;
    while (i < s.length()) {
        while (isspace(s[i]) && (i < s.length())) {
            temp.push_back(s[i]);
            ++i;
        }
        if (temp != "") {
            result.push_back(temp);
            temp = "";
        }
        while (!isspace(s[i]) && (i < s.length())) {
            temp.push_back(s[i]);
            ++i;
        }
        if (temp != "") {
            result.push_back(temp);
            temp = "";
        }
    }
    return result;
}
    

