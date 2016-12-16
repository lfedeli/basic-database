/*
Implementation of a database with the functions
1-SET  2-UNSET  3-GET  4-NUMEQUALTO  5-BEGIN  6-ROLLBACK  7-COMMIT  8-END
In order to have a performance of O(logN) with N the number of inputs I have defined 2 data structures:
a binary search tree in which I have organized the values (this is in particular to have
good performance for the function NUMEQUALTO), and an hash table, with a modified key generator
to avoid conflicts (good performance for the function GET). 
For the hash table the performance of the operations is O(1), with a worste case O(N);
anyway, especially when the size of the hashtable is much bigger that the number of inputs, the access
is, as mentioned O(1).

In order to implement the operations BEGIN, ROLLBACK, COMMIT I have implemented the arrays
"T_transaction" and  "H_transaction" that handle the openings of different transactional blocks.  


*/
#include <iostream>
#include <cstdio>
#include <fstream>
#include <string.h>

using namespace std;


class database
{

public:
ofstream myfile, myfile1;
struct cell {
double value, numbe;
cell* parent;
cell* left_leave;
cell* right_leave;
       };

struct couple{
string key;
double value;
};
	   cell* T;  
	   couple* H;
           unsigned int m_size; 
	   cell** T_transaction;  
	   couple** H_transaction;  
	   bool transaction_activated;
	   int transaction_n;
	   database(int N, unsigned int M):m_size(M) { T = 0;                             // N is the maximum number of transactional block I can handle, M size of the hash table
                           transaction_n = -1;
	                   H = new couple [m_size];
                           transaction_activated = false;
	                   T_transaction = new cell* [N];
			   H_transaction = new couple* [N];
                           for (unsigned int i = 0; i<(unsigned int)N; i++)
					  {         
					   T_transaction[i] = 0;
					   H_transaction[i] = new couple [m_size];
					  }
                           transaction_activated = false;	   
                      }



			   ~database() {  }



//#################################################### ROUTINES TO HANDLE THE BINARY SEARCH TREE #######################
				  
                          void  treeInsert(double value_i, cell** T)
                            {
                              cell* y;
                              y = NULL;
                              cell* x;
                              x = *T;
                                 while (x != 0)
                                        {
                                             y = x;
                                             if (value_i == x->value) break;
                                             if (value_i < x->value)
                                                     x = x->left_leave;
                                              else x = x->right_leave;
                                        }
                                    if (y == 0) 
                                             { 
                                                y = new cell;
                                                y->value=value_i; 
                                                y->left_leave=NULL; y->right_leave = NULL; 
                                                y->parent = NULL;
                                                y->numbe = 1;
                                                *T = y;
                                           }
                                    else
                                       {
                                        if (value_i == y->value)
                                          {
                                                y->numbe += 1;
                                          }
                                        else
                                           {
                                              cout << "sono in value else" << endl;
                                              cell* z;
                                              z = new cell;
                                              z->numbe = 1;
                                              z->value = value_i; z->left_leave=NULL; z->right_leave = NULL;   z->parent = y;
                                              if (value_i < y->value)
                                                { y->left_leave = z;}
                                               else  
                                                 {
                                                y->right_leave = z;
                                                 }
                                            }
                                       }
                            }

                          cell* treeSearch(double value_i, cell* T)
                              {
                                if (T == NULL || T->value == value_i)
                                    return T;
                                else
                                   if (value_i < T->value)
                                          treeSearch(value_i, T->left_leave);
                                  else
                                          treeSearch(value_i, T->right_leave);

                              }

                          cell* treeMinimum(cell* x)
                                {
                                 cell* a = x;
                                 while (a->left_leave != NULL)
                                   a = a->left_leave;
                                  return a;
                                }

                            cell* treeMaximum(cell* x)
                                {
                                 cell* a = x;
                                 while (a->right_leave != NULL)
                                   a = a->right_leave;
                                  return a;
                                 }

                        cell* treeSuccessor(cell* x)
                             {
                              if (x->right_leave != NULL)
                              return treeMinimum(x->right_leave); 
                              else
                              return treeMaximum(x->left_leave); 
                             }

                        cell* treePredecessor(cell* x)
                             {
                              if (x->left_leave != NULL)
                               return treeMaximum(x->left_leave);
                              cell* y;
                               y = x->parent;
                                while (y != NULL && x == y->left_leave)
                                  {
                                    x = y;
                                    y = y->parent;
                                  }
                               return y;
                             }

	                 void destroy(cell** item)      //clean the tree 
                           {

                             if (*item != NULL)
                             {
                               destroy(&((*item)->left_leave));
                               destroy(&((*item)->right_leave));
                               (*item)->left_leave= NULL;
                               (*item)->right_leave = NULL;
                               (*item)->parent = NULL;
                                *item = NULL;
                       //        delete *item;
                             }
                           }

// ####################################### TREE DELETE ################################à

                        cell* treeDelete(cell** T, double value_i)
                              {
                              cell* z;
                              cell* y;
                              cell* x;
                              z = treeSearch(value_i,*T);
                               if (z->numbe == 1)
                                 {
                                      if (z->left_leave == NULL && z->right_leave == NULL)
                                       y = z;
                                       else 
                                       y = treeSuccessor(z);


                                       if (y->left_leave != NULL)                        
                                         x = y->left_leave;
                                       else
                                       x= y->right_leave;

                                       if (x != NULL)
                                         x->parent = y->parent;

                                        if (y->parent == NULL)
                                           *T = x;
                                        else 
                                          if (y == y->parent->left_leave)
                                              y->parent->left_leave = x;
                                          else
                                              y->parent->right_leave = x;
                                         if (y != z) 
                                          {
                                             if (z != *T)
                                             z->value=y->value;
                                             else
                                             (*T)->value=y->value;
                                           }
                                        }
                                 else{
                                      if (z != *T)
                                      z->numbe -= 1;       
                                      else
                                      (*T) -> numbe -= 1;
                                      }
                                 return y;
                             }

//###################################################àà HASH FUNCTION GENERATING KEY FOR INSERTION ##################################################

                            unsigned int convertHash(string key, unsigned int slot) {
                               unsigned int hash = 5381;

                                for (unsigned int i = 0; i < key.length(); i++)
                                      hash = ((hash << 5) + hash) + (unsigned int)key[i];

                               return (hash + slot) % m_size;
                               }

///######################################################  SET FUNCTION ########################################################################
 		             void set(string name_i, double value_i)
				       {
                                        unsigned  int k;
					 if (transaction_activated == false)
					    {
                                               treeInsert(value_i,&T);
                                              unsigned  int slot = 0;
                                                   while (slot < m_size)
                                                    {
                                                        k = convertHash(name_i,slot);
                                                           if (H[k].value == 0)
                                                              {
                                                                H[k].key = name_i;
                                                                H[k].value = value_i;
                                                                 cout << "Ho inserito in hashtable" << endl;
                                                                break;
                                                                }
                                                        slot = slot +1;
                                                        cout << "WARNING: Incremented slot" << endl;
                                                    }
                                                if (slot == m_size)
                                                cout << "I have exceeded the size of hash table" << endl;
						 }
					 else 					
					    {
                                                treeInsert(value_i,&T_transaction[transaction_n]);
                                                unsigned  int slot = 0;
                                                   while (slot < m_size)
                                                    {
                                                         k = 0;
                                                         k = convertHash(name_i,slot);
                                                           if (H_transaction[transaction_n][k].value == 0)
                                                              {
                                                                H_transaction[transaction_n][k].key = name_i;
                                                                H_transaction[transaction_n][k].value = value_i;
                                                                break;
                                                                }
                                                        slot = slot +1;
                                                     }
                                                if (slot == m_size)
                                               cout << "I have exceeded the size of hash table" << endl;
                 		            }
                                           }

//##################################################################	UNSET ########################################################################
                                 void unset(string name_u)
                                       {
                                       unsigned int k;
                                       cell* x;
                                       double value_parz;
                                       unsigned  int slot = 0;
                                        if (transaction_activated == false)
                                         {
                                                   while (slot < m_size)
                                                    {
                                                        k = convertHash(name_u,slot);
                                                           if ((H[k].key).compare(name_u) == 0)
                                                              {
                                                                value_parz = H[k].value;
                                                                H[k].key = " ";
                                                                H[k].value = 0; 
                                                                break;
                                                               }
                                                         slot = slot +1;
                                                     }
                                                         if (slot == m_size )
                                                         cout << "No variable with name "  << name_u << endl;

                                                    cout << "value to be canceled " << value_parz <<endl;
                                                    x = treeDelete(&T,value_parz);
                                         }
                                        else
                                         {
                                                   while (slot < m_size)
                                                    {
                                                        k = convertHash(name_u,slot);
                                                           if ((H_transaction[transaction_n][k].key).compare(name_u) == 0)
                                                              {
                                                                value_parz = H_transaction[transaction_n][k].value;
                                                                H_transaction[transaction_n][k].key = " ";
                                                                H_transaction[transaction_n][k].value= 0; 
                                                                break;
                                                               }
                                                         slot = slot +1;
                                                     }
                                                         if (slot == m_size)
                                                         cout << "No variable with name "  << name_u << endl;

                                         cout << "valore da cancellare " << value_parz <<endl;
                                          x = treeDelete(&T_transaction[transaction_n],value_parz);
                                         }
                                       }



// ####################################################### GET FUNCTION ##################################################
			  void get(string name_i)
					{
                                         unsigned int k;
                                         double value_parz;	
                                         unsigned  int slot = 0;				  
					  if (transaction_activated == false)
                                         { 
                                                   while (slot < m_size)
                                                    {
                                                        k = convertHash(name_i,slot);
                                                           if ((H[k].key).compare(name_i) == 0)
                                                              {
                                                                cout << "The name is " << H[k].key << endl; 
                                                                value_parz = H[k].value;
                                                                cout << "The value is " << value_parz << endl;
                                                                break;
                                                               }
                                                         slot = slot +1;                           
                                                     }
                                                         if (slot == m_size)
                                                         cout << "No variable with name "  << name_i << endl;
	                                 }
  
					  else
                                           {
                                                   while (slot < m_size)
                                                    {
                                                        k = convertHash(name_i,slot);
                                                           if ((H_transaction[transaction_n][k].key).compare(name_i) == 0)
                                                              {
                                                                value_parz = H_transaction[transaction_n][k].value;
                                                                cout << "The value is " << value_parz << endl;
                                                                break;
                                                               }
                                                         slot = slot +1;
                                                     }
                                                         if (slot == m_size )
                                                         cout << "No variable with name "  << name_i << endl;
                                            }
					}





//########################################################À    NUMEQUALTO   ######################################################
					int numequalto(double value_i)
					{
					        cell* c1;
					        int num;
	                                          if (transaction_activated == false)
                                                   { c1 =  treeSearch(value_i,T);}
					          else
                                                  c1 = treeSearch(value_i,T_transaction[transaction_n]);
                                                  if (c1 == NULL)
					           {
                                                     cout << "no variable with value "<< value_i << endl;
                                                     num = 0;
                                                   }
                                                 else
                                                   num = c1->numbe;		
                      					  
					            return num;
					}

//###################################################  	BEGIN TRANSACTION #######################################################

					void begin_t()  
					{
					   int i = 0;
					   transaction_activated = true;
					   while ( T_transaction[i] != NULL)
						   i++;

					   transaction_n = i;  // I am opening the ith transactional block
                                         cout << "the number of transactional block is "<< transaction_n << endl;
					
					}

//##############################################à ROLLBACK #######################################################################À

					void roll_back_t()  
					{

					  if (transaction_n > -1)   // I am going back in T-transactional
					  {  
						  destroy(&(T_transaction[transaction_n]));
                                                  for (unsigned int j = 0; j < m_size; j++)
                                                    {
                                                        if(H_transaction[transaction_n][j].value != 0)
                                                         {
                                                            H_transaction[transaction_n][j].key = " ";
                                                            H_transaction[transaction_n][j].value = 0;
                                          
                                                         }
                                                     }
 						  transaction_n  -= 1;
					             if (transaction_n == -1)
                                                        {
						           transaction_activated = false;
                                                         }
					  }
					  else
                                               {
						  cout << "ERROR: no transactional block open" << endl;
                                               }

					 }

//################################################################### COMMIT (WITH 2 HELPING ROUTINES) ####################################

					void insert_perm(cell* item)  //insert permanently in the main tree
					{
                                         double value_insp;
                                         double nume;
					 if (item != NULL)
					    {
					   insert_perm(item->left_leave);
					   insert_perm(item->right_leave);
					   value_insp = item->value;
                                           nume = numequalto(value_insp); 
                                               for (unsigned int j=0; j < nume ; j++)
                                                 {
					             treeInsert(value_insp,&T);  
                                                 }
				              }
                                            }

					 void insert_perm_hash(couple* H_insp)  //insert permanently in the main hash table
					 {
                                           double value_parz;
                                           string key_parz;
                                           for (unsigned int j=0; j<m_size; j++)
                                              {
                                                 if (H_insp[j].value != 0)
                                                    {
                                                      value_parz = H_insp[j].value;
                                                      key_parz = H_insp[j].key;
                                                      unsigned int slot =0;
                                                      unsigned int k =0;
                                                          while (slot < m_size)
                                                             {
                                                               k = convertHash(key_parz,slot);
                                                               if (H[k].value == 0)
                                                                 {
                                                                    H[k].key = key_parz;
                                                                    H[k].value = value_parz;
                                                                    break;
                                                                  }
                                                                  slot = slot +1;
                                                             }
                                                       if (slot == m_size)
                                                       cout << "no space for insert " << endl;
                                                       }
                                                  }
                                          }


                                        void commit_t()
                                           {
                                              if (transaction_n > -1)
                                                {
                                                cout << "I AM COMMITTING THE OPERATIONS" << endl;
                                                   insert_perm(T_transaction[transaction_n]);
                                                   insert_perm_hash(H_transaction[transaction_n]);
                                                   roll_back_t();
                                                }
                                               else
                                                cout << "ERROR: no transactional block activate" << endl;

                                           }


    
                                void printsufile(cell* item, int cont)
                                    {
                                       if (cont == 0)
                                       myfile.open("stampa.dat");
                                       if (item != 0)
                                         {
                                           cont += 1;
                                           printsufile(item->right_leave, cont);
                                           printsufile(item->left_leave, cont);
                                           myfile << "value = " << item-> value;
                                           myfile << "number of values  = " << item->numbe;
                                           if (item->right_leave != 0)
                                           myfile << "right leave = " << item->right_leave->value;
                                           if (item->left_leave != 0)
                                           myfile << "  left leave = " << item->left_leave->value;
                                           if (item -> parent != 0)
                                           myfile << "  parent = " << item->parent->value << endl;
                                           else
                                           myfile << "  " <<  endl;
                                         }
                                      }


                                 void printhashtable(couple* H){
                                 myfile1.open("hash.dat");
                                 for (unsigned int i = 0; i < m_size; i++)
                                   myfile1 << "i = " << i << " key = " << H[i].key << "  valore " << H[i].value << endl;
                                   }

};

//Main Function
int main()
{


int size;
cout << "Please, digit the maximum number of elements you are planning to insert" << endl;
cin >> size;
database database_mio(20,size);

string name_main;
double value_main = 0;
char quest='y';
int code_op;
while (quest == 'y'){
cout << "Please, enter the number of operation you would like to do: " << endl;
cout << "1-SET  2-UNSET  3-GET  4-NUMEQUALTO  5-BEGIN  6-ROLLBACK  7-COMMIT  8-END " << endl;
cin >> code_op;
switch ( code_op ) {
  case 1:  
    cout << "Please, enter the name of the variable you want to insert" << endl;     
	while (name_main.empty())
    getline(cin,name_main);
    cout << "Please, enter its value" << endl;
    cin >> value_main;       
    database_mio.set(name_main,value_main);
	name_main.clear();
        value_main = 0;
    break;
  case 2:        
    cout << "Please, enter the name of the variable you want to unset" << endl;     
	while (name_main.empty())
    std::getline (std::cin,name_main);  
    database_mio.unset(name_main);
	name_main.clear();
    break;
  case 3:        
    cout << "Please, enter the name to get" << endl;  
    while (name_main.empty())
    std::getline (std::cin,name_main); 
    database_mio.get(name_main);
	name_main.clear();
    break;
  case 4:      
   int value;
    cout << "Please, enter the value" << endl; 
    cin >> value_main;
    value = database_mio.numequalto(value_main);
 cout << "number of equals " << value << endl;
   break;
  case 5:
   cout << "start BEGIN BLOCK" << endl;      
   database_mio.begin_t();
   break;
   case 6:      
   cout << "doing ROLLBACK" << endl;    
   database_mio.roll_back_t();
   break;
   case 7:      
   cout << "COMMIT" << endl;    
   database_mio.commit_t();
   break;
   case 8:      
   quest = 'n';
   break;
}

}

    return 0;
}



