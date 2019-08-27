#include <cxxtest/TestSuite.h>
#include "CellProperties.hpp"
#include "SteadyStateRunner.hpp"
#include "AbstractCvodeCell.hpp"
#include "RegularStimulus.hpp"
#include "EulerIvpOdeSolver.hpp"
#include "Shannon2004Cvode.hpp"
#include "FakePetscSetup.hpp"
#include "SimulationTools.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

/* These header files are generated from the cellml files provided at github.com/chaste/cellml */

#include "beeler_reuter_model_1977Cvode.hpp"
#include "ten_tusscher_model_2004_epiCvode.hpp"
#include "ohara_rudy_2011_endoCvode.hpp"
#include "shannon_wang_puglisi_weber_bers_2004Cvode.hpp"
#include "decker_2009Cvode.hpp"

class TestGroundTruthSimulation : public CxxTest::TestSuite
{
private:
  const double threshold = 1.5e-07;
  
public:
  void TestTusscherSimulation()
  {
#ifdef CHASTE_CVODE
    boost::shared_ptr<RegularStimulus> p_stimulus;
    boost::shared_ptr<AbstractIvpOdeSolver> p_solver;

    std::vector<boost::shared_ptr<AbstractCvodeCell>> models;
    bool threshold_met = false;
      
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Cellohara_rudy_2011_endoFromCellMLCvode(p_solver, p_stimulus)));
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Celldecker_2009FromCellMLCvode(p_solver, p_stimulus)));
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Cellten_tusscher_model_2004_epiFromCellMLCvode(p_solver, p_stimulus)));
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Cellshannon_wang_puglisi_weber_bers_2004FromCellMLCvode(p_solver, p_stimulus)));
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Cellohara_rudy_2011_endoFromCellMLCvode(p_solver, p_stimulus)));
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Celldecker_2009FromCellMLCvode(p_solver, p_stimulus)));
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Cellten_tusscher_model_2004_epiFromCellMLCvode(p_solver, p_stimulus)));
    models.push_back(boost::shared_ptr<AbstractCvodeCell>(new Cellshannon_wang_puglisi_weber_bers_2004FromCellMLCvode(p_solver, p_stimulus)));

    std::string username = std::string(getenv("USER"));
    boost::filesystem::create_directory("/tmp/"+username);

    std::ofstream output_file;
    output_file.open("/tmp/"+username+"/benchmarks.txt");
    
    for(unsigned int i = 0; i < models.size(); i++){
      double period = 1000;
      if(i<4)
	period = 500;
      boost::shared_ptr<AbstractCvodeCell> p_model = models[i];
      boost::shared_ptr<RegularStimulus> p_regular_stim = p_model->UseCellMLDefaultStimulus();
      
      const std::string model_name = p_model->GetSystemInformation()->GetSystemName();
      std::cout << "Testing " << model_name << " with period " << period << "\n";
      boost::filesystem::create_directory("/tmp/"+username);

      const double duration   = p_regular_stim->GetDuration();

      if(period==500){
      	TS_ASSERT_EQUALS(LoadStatesFromFile(p_model, "/home/joey/code/chaste-project-data/"+model_name+"/GroundTruth1Hz/final_state_variables.dat"), 0);
      }
      else{
	TS_ASSERT_EQUALS(LoadStatesFromFile(p_model, "/home/joey/code/chaste-project-data/"+model_name+"/GroundTruth2Hz/final_state_variables.dat"), 0);
      }
      
      p_regular_stim->SetPeriod(period);
      p_regular_stim->SetStartTime(0);
      p_model->SetTolerances(1e-8, 1e-8);
      p_model->SetMaxSteps(1e5);
      
      const unsigned int paces  = 5000;
      OdeSolution current_solution;
      const std::vector<std::string> state_variable_names = p_model->rGetStateVariableNames();
      
      p_model->SetMaxTimestep(1000);
      
      TS_ASSERT_EQUALS(output_file.is_open(), true);

      /*Set the output to be as precise as possible */
      output_file.precision(18);
      
      /*Run the simulation*/
      std::vector<double> current_states, previous_states;
      threshold_met = false;
      for(unsigned int i = 0; i < paces - 1; i++){
	if(i>0)
	  previous_states = current_states;
	p_model->SolveAndUpdateState(0, duration);
	p_model->SolveAndUpdateState(duration, period);
	current_states = p_model->GetStdVecStateVariables();
	if(i>0){
	  if(mrms(current_states, previous_states) < threshold){
	    threshold_met = true;
	    output_file << model_name << " with period " << period << " reached the threshold " << threshold << " after " << i << "paces.\n";
	    break;
	  }
	}
      }
      TS_ASSERT(threshold_met);
    }
#else
    std::cout << "Cvode is not enabled.\n";
#endif
  }
};