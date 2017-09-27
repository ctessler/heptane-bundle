#ifndef DOTFROMCFRG_H
#define DOTFROMCFRG_H

class DOTfromCFRG {
public:
	DOTfromCFRG(CFRG &cfrg) : _cfrg(cfrg) { }
	/* Gets and sets the path */
	string getPath() { return _path; }
	void setPath(string path) { _path = path; }

	void produce(unsigned int threads=1);
private:
	CFRG &_cfrg;

	string _path;
};

#endif
