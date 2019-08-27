#include <boost/filesystem.hpp>
#include <fstream>
#include <cxxtest/TestSuite.h>
#include "CellProperties.hpp"
#include "SteadyStateRunner.hpp"
#include "AbstractCvodeCell.hpp"
#include "RegularStimulus.hpp"
#include "EulerIvpOdeSolver.hpp"
#include "Shannon2004Cvode.hpp"
#include "FakePetscSetup.hpp"
#include "SimulationTools.hpp"
/* These header files are generated from the cellml files provided at github.com/chaste/cellml */

#include "beeler_reuter_model_1977Cvode.hpp"
#include "ten_tusscher_model_2004_epiCvode.hpp"
#include "ohara_rudy_2011_endoCvode.hpp"
#include "shannon_wang_puglisi_weber_bers_2004Cvode.hpp"
#include "decker_2009Cvode.hpp"

class TestGroundTruthSimulation : public CxxTest::TestSuite
{
private:
  const unsigned int buffer_size = 150;
 
  std::vector<std::vector<double>> LogDifferences(std::vector<double> &values){
    std::vector<double> x_vals, y_vals;
    x_vals.reserve(buffer_size);
    y_vals.reserve(buffer_size);
    for(unsigned int i=0; i<values.size()-1; i++){
      double tmp  = abs(values[i] - values.back());
      if(tmp != 0){
	tmp = log(tmp);
	y_vals.push_back(tmp);
	x_vals.push_back(i);
      }
    }
    return std::vector<std::vector<double>>({x_vals, y_vals});
  }
  
  double CalculatePMCC(std::vector<std::vector<double>> values){
    const unsigned int N = values.size();
    // const double sum_x = N*(N-1)/2;
    // const double sum_x2 = (N-1)*N*(2*N-1)/6;
    double sum_x = 0, sum_x2 = 0, sum_y = 0, sum_y2 = 0, sum_xy = 0;
    
    for(unsigned int i = 1; i <= N; i++){
      sum_x  += values[0][i];
      sum_x2 += values[0][i]*values[0][i];
      sum_y  += values[1][i];
      sum_y2 += values[1][i]*values[1][i];
      sum_xy += values[0][i]*values[1][i];
    }

    double pmcc = (N*sum_xy - sum_x*sum_y)/sqrt((N*sum_x2 - sum_x*sum_x)*(N*sum_y2 - sum_y*sum_y));
    
    if(abs(pmcc)>1){
      std::cout << pmcc << " ";
      for(unsigned int i = 0; i < buffer_size; i++){
	std::cout << values[1][i] << " ";
      }
      std::cout << "\n";
    }

    if(abs(pmcc)>0.8){
      std::cout << pmcc << " " << "\n";
    }
    
    TS_ASSERT(abs(pmcc <= 1.001));

    return pmcc;
  }
  
public:
  void TestTusscherSimulation()
  {
#ifdef CHASTE_CVODE
    boost::shared_ptr<RegularStimulus> p_stimulus;
    boost::shared_ptr<AbstractIvpOdeSolver> p_solver;

    std::vector<boost::shared_ptr<AbstractCvodeCell>> models;
    
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
    
    for(unsigned int i = 1; i < 2; i++){
      double period = 1000;
      if(i<4)
	period = 500;

      boost::shared_ptr<AbstractCvodeCell> p_model = models[i];
      boost::shared_ptr<RegularStimulus> p_regular_stim = p_model->UseCellMLDefaultStimulus();
      
      const std::string model_name = p_model->GetSystemInformation()->GetSystemName();
      boost::filesystem::create_directory("/tmp/"+username+"/"+model_name);

      const double duration   = p_regular_stim->GetDuration();

      p_regular_stim->SetPeriod(period);
      p_regular_stim->SetStartTime(0);
      p_model->SetTolerances(1e-8, 1e-8);
      p_model->SetMaxSteps(1e5);
      
      const unsigned int paces = 5000;
      OdeSolution current_solution;
      std::ofstream output_file;
      std::string errors_file_path;
      
      const std::vector<std::string> state_variable_names = p_model->rGetStateVariableNames();

      if(period == 500){
	TS_ASSERT_EQUALS(LoadStatesFromFile(p_model, "/home/joey/code/chaste-project-data/"+model_name+"/GroundTruth1Hz/final_state_variables.dat"), 0);
	boost::filesystem::create_directory("/tmp/"+username+"/"+model_name);      
	errors_file_path = "/tmp/"+username+"/"+model_name+"/1Hz2Hzerrors.dat";
      }
      else{
	TS_ASSERT_EQUALS(LoadStatesFromFile(p_model, "/home/joey/code/chaste-project-data/"+model_name+"/GroundTruth2Hz/final_state_variables.dat"), 0);
	boost::filesystem::create_directory("/tmp/"+username+"/"+model_name);      
	errors_file_path = "/tmp/"+username+"/"+model_name+"/2Hz1Hzerrors.dat";
      }

      std::cout << "Testing " << model_name << " with period " << period << "\n";
      p_model->SetMaxTimestep(1000);
      if(period==500){
	output_file.open("/tmp/"+username+"/"+model_name+"/pmcc1Hz2Hz.dat");
      }
      else{
	output_file.open("/tmp/"+username+"/"+model_name+"/pmcc2Hz1Hz.dat");
      }
      TS_ASSERT_EQUALS(output_file.is_open(), true);
      
      /*Set the output to be as precise as possible */
      output_file.precision(18);

      boost::circular_buffer<std::vector<double>> values(buffer_size);            
    
      /*Run the simulation*/

      std::vector<double> current_state_variables = p_model->GetStdVecStateVariables(), previous_state_variables;
      for(unsigned int i = 0; i < paces; i++){
	previous_state_variables = current_state_variables;
	p_model->SolveAndUpdateState(0, duration);
	p_model->SolveAndUpdateState(duration, period);
	current_state_variables = p_model->GetStdVecStateVariables();

	double mrms_val = mrms(current_state_variables, previous_state_variables);
	/*If values is full, this will remove the oldest value so that values will only contain up to buffer_size points*/
	values.push_back(current_state_variables);
	values.back().push_back(mrms_val);

	if(i>buffer_size && i % 20 == 0){
	  for(unsigned int i = 0; i < values.size(); i++){
	    std::vector<double> state_trace = cGetNthVariable(values, i);
	    std::vector<std::vector<double>> values = LogDifferences(state_trace);
	    output_file << CalculatePMCC(values) << " ";
	  }
	  output_file << "\n";
	}
      }
      output_file.close();
    }
#else
    std::cout << "Cvode is not enabled.\n";
#endif
   }
};
