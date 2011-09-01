//-----------------------------------------------------------------------------
// t-eoinclusion.cpp
//-----------------------------------------------------------------------------

#include <ga/eoBin.h>
#include <eoPop.h>
#include <eoInclusion.h>

//-----------------------------------------------------------------------------

typedef eoBin<float> Chrom;

#include "binary_value.h"

//-----------------------------------------------------------------------------

main()
{
  const unsigned CHROM_SIZE = 4;

  eoBinRandom<Chrom> random;
  BinaryValue eval;

  for (unsigned POP_SIZE = 4; POP_SIZE <=6; POP_SIZE++)
    {
      eoPop<Chrom> pop; 
      unsigned i;
      for ( i = 0; i < POP_SIZE; i++)
	    {
	      Chrom chrom(CHROM_SIZE);
	      random(chrom);
	      eval(chrom);
	      pop.push_back(chrom);
	    }
      
      for (unsigned POP2_SIZE = 4; POP2_SIZE <=6; POP2_SIZE++)
	{
	  eoPop<Chrom> pop2, pop3, pop4, pop5;
	 	
	  for (i = 0; i < POP2_SIZE; i++)
	    {
	      Chrom chrom(CHROM_SIZE);
	      random(chrom);
	      eval(chrom);
	      pop2.push_back(chrom);
	    }
	
	  std::cout << "--------------------------------------------------" << std::endl
	       << "breeders \tpop" << std::endl
	       << "--------------------------------------------------" << std::endl;
	  for (i = 0; i < max(pop.size(), pop2.size()); i++)
	    {	  
	      if (pop.size() > i) 
		std::cout << pop[i] << " " << pop[i].fitness() << "   \t";
	      else
		std::cout << "\t\t";
	      if (pop2.size() > i)
		std::cout << pop2[i] << " " << pop2[i].fitness();
	      std::cout << std::endl;
	    }
	
	  eoInclusion<Chrom> inclusion(0.75);
	  pop3 = pop2;
	  inclusion(pop, pop3); 
	  
	  eoInclusion<Chrom> inclusion2; 
	  pop4 = pop2;
	  inclusion2(pop, pop4); 
	  
	  eoInclusion<Chrom> inclusion3(1.5);
	  pop5 = pop2;
	  inclusion3(pop, pop5); 
	  
	  std::cout << std::endl
	       << "0.75 \t\t1.0 \t\t1.5" << std::endl
	       << "---- \t\t--- \t\t---" << std::endl;
	  for (i = 0; i < pop5.size(); i++)
	    {
	      if (pop3.size() > i)
		std::cout << pop3[i] << " " << pop3[i].fitness() << "   \t";
	      else
		std::cout << " \t\t";
	      if (pop4.size() > i)
		std::cout << pop4[i] << " " << pop4[i].fitness() << "   \t";
	      else
		std::cout << " \t\t";
	      if (pop5.size() > i)
		std::cout << pop5[i] << " " << pop5[i].fitness();
	      std::cout << std::endl;
	    }
	}
    }
  
  return 0;
}

//-----------------------------------------------------------------------------
