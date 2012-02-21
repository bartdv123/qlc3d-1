#include <refinfo.h>
#include <assert.h>

void RefInfo::setType(std::string s)
{
// TESTS WHETHER "REFINEMENT.Type" STRING IN SETTNGS FILE IS VALID.
// IF YES, SETS type_ ENUMERATOR FOR THIS OBJECT
// IF NOT, TERMINATES PROGRAM

    // MAKE LOWERCASE
    std::transform(s.begin(), s.end(), s.begin(), std::ptr_fun<int, int>(std::tolower));

    if ( ! s.compare("change") )
    {
        type_ = Change;
        return;
    }
    else
        if ( ! s.compare("sphere") )
        {
            type_ = Sphere;
            return;
        }
    else
            if( ! s.compare("box") )
        {
            type_ = Box;
            return;
        }
    else
    {
        std::cout<< "error - bad refinement type: "<< s << " bye!" << std::endl;
        exit(1);
    }
}
void RefInfo::setRefIter()
{
// DETERMINES NUMBER OF REFINEMENT ITERATIONS THIS OBJECT DESCRIBES
// AND SETS THE refIter_ VARIABLE

// DEPENDING ON THE type_, NUMBER OF REFINEMENT ITERATIONS IS DETERMINED BY
// DIFFERENT CONDITIONS
    switch(type_)
    {
    case(Change):
    {
        refIter_= (int) values_.size();
        break;
    }
    case (Sphere):
    {
        refIter_ = (int) values_.size();
        // MAKE SURE NOT EMPTY AND EQUAL NUMBER OF COORDS HAVE BEEN SPECIFIED
        break;
    }
    case (Box):
    {
        // FOR A BOX, refIter DEPENDS ON NUMBER OF COORDS SPECIFIED
        size_t numx = X_.size();
        refIter_ = (int) numx / 2;
        break;
    }
    default:
        printf("error in %s, unhandled refinement type - bye\n", __func__);
        exit(1);
    }
}


RefInfo::RefInfo(const std::string& Type):
    type_(None),
    iter_(0),
    time_(0),
    refIter_(0),
    X_(1,0),
    Y_(1,0),
    Z_(1,0)

{
    setType(Type);

}

RefInfo::RefInfo(const RefInfo &other):
    type_(other.type_),
    iter_(other.iter_),
    time_(other.time_),
    refIter_(other.refIter_),
    values_(other.values_),
    X_(other.X_),
    Y_(other.Y_),
    Z_(other.Z_)
{

}



void RefInfo::setValues(std::vector<double> &values)
{
    values_.clear();
    values_.insert( values_.begin(), values.begin(), values.end() );
    this->setRefIter();
}



void RefInfo::setCoords(const std::vector<double> &x,
                        const std::vector<double> &y,
                        const std::vector<double> &z)
{
    X_.clear();
    Y_.clear();
    Z_.clear();

    X_.insert(X_.begin(), x.begin(), x.end() );
    Y_.insert(Y_.begin(), y.begin(), y.end() );
    Z_.insert(Z_.begin(), z.begin(), z.end() );

    this->setRefIter();

}


void RefInfo::getCoords(std::vector<double> &x,
                        std::vector<double> &y,
                        std::vector<double> &z) const
{
    x = X_;
    y = Y_;
    z = Z_;
}

void RefInfo::getCoord(double &x, double &y, double &z) const
{
    assert( X_.size() );
    assert( Y_.size() );
    assert( Z_.size() );

    x = X_[0];
    y = Y_[0];
    z = Z_[0];
}

double RefInfo::getValue(const size_t i) const
{
#ifdef DEBUG
    assert ( i <= values_.size() );
#endif
    return values_[i];
}

void RefInfo::printRefInfo(FILE *fid) const
{
    //fprintf(fid, "Iteration = %ll\n", this->iter_);
    std::cout << "Iteration = " << this->iter_ << std::endl;
    //fprintf(fid, "Time = %e\n", this->time_);
    std::cout << "Time = "<< this->time_ << std::endl;
    //fprintf(fid, "Type = %i\n", this->type_);
    std::cout << "Type = " << this->type_ << std::endl;
    for (size_t i = 0 ; i < values_.size() ; i++)
        std::cout <<"values["<<i<<"]="<<values_[i]<<std::endl;
        //fprintf(fid,"values_[%u] = %e\n", i , values_[i]);

}

void RefInfo::validate(const RefInfo &refinfo)
{
#define ERRORMSG printf("\n\nerror, bad or missing REFINEMENT data - bye!\n")
    switch ( refinfo.getType() )
    {
    case( RefInfo::Change ):
        // MAKE SURE VALUES EXIST
        if (!refinfo.values_.size() )
        {
            ERRORMSG;
            exit(1);
        }
        break;
    case( RefInfo::Sphere ):
    {
        // MAKE SURE VALUES AND COORDINATES EXIST
        if ( ( !refinfo.values_.size() ) ||
             ( !refinfo.X_.size() ) ||
             ( !refinfo.Y_.size() ) ||
             ( !refinfo.Z_.size() ) )
        {
             ERRORMSG;
             exit(1);
        }
        break;
    }
    case( RefInfo::Box ):
    {
        // MAKE SURE CORRECT NUMBER OF X,Y AND Z COORDINATES HAVE BEEN SPECIFIED
        size_t numx = refinfo.X_.size();
        size_t numy = refinfo.Y_.size();
        size_t numz = refinfo.Z_.size();
        if ( (!numx) || // COORDINATES MUST BE DEFINED
             (!numy) ||
             (!numz))
        {
            ERRORMSG;
            exit(1);
        }
        if ( (numx != numy )||  // EQUAL NUMBER OF COORDS. DEFINED
             (numx != numz ) )
        {
            ERRORMSG;
            exit(1);
        }
        if ( (numx%2 != 0) || // TWO COORDS PER BOX
             (numy%2 != 0) ||
             (numz%2 != 0) )
        {
            ERRORMSG;
            exit(1);
        }
        break;
    }
    case( RefInfo::None ):
        printf("RefInfo type is None - bye!");
        exit(1);
        break;
    default:
        printf("error in %s, unknonwn refinement type - bye!\n",__func__);
        exit(1);
    }


}