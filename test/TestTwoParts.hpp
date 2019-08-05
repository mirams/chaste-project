#include <cxxtest/TestSuite.h>
#include "CellProperties.hpp"
#include "SteadyStateRunner.hpp"
#include "AbstractCvodeCell.hpp"
#include "RegularStimulus.hpp"
#include "EulerIvpOdeSolver.hpp"
#include "Shannon2004Cvode.hpp"
#include "FakePetscSetup.hpp"
#include <fstream>

#include "beeler_reuter_model_1977Cvode.hpp"
#include "ten_tusscher_model_2004_epiCvode.hpp"
#include "ohara_rudy_2011_endoCvode.hpp"
#include "shannon_wang_puglisi_weber_bers_2004Cvode.hpp"

class TestGroundTruthSimulation : public CxxTest::TestSuite
{
public:
    void TestTusscherSimulation()
    {
#ifdef CHASTE_CVODE
        boost::shared_ptr<RegularStimulus> p_stimulus;
        boost::shared_ptr<AbstractIvpOdeSolver> p_solver;
	boost::shared_ptr<AbstractCvodeCell> p_model(new Cellohara_rudy_2011_endoFromCellMLCvode(p_solver, p_stimulus));
	boost::shared_ptr<RegularStimulus> p_regular_stim = p_model->UseCellMLDefaultStimulus();
	
	int paces = 1000;
	OdeSolution current_solution;
	const double period = 1000;
	
        p_model->SetMaxTimestep(1000);
	
        p_regular_stim->SetPeriod(period);
     	p_model->SetTolerances(1e-10,1e-10);
	p_model->SetMaxSteps(1e5);
	double start_time = p_regular_stim->GetStartTime();
	double duration   = p_regular_stim->GetDuration();
	
	for(int i=0; i < paces; i++){
	  if(start_time>0)
	    p_model->SolveAndUpdateState(0, start_time);
	  p_model->SolveAndUpdateState(start_time, start_time + duration);
	  p_model->SolveAndUpdateState(start_time + duration, period);
	}
	p_model->SolveAndUpdateState(0, start_time);
	p_model->SolveAndUpdateState(start_time, start_time + duration);
	OdeSolution solution = p_model->Compute(start_time + duration, period, 1);
	solution.WriteToFile("1PartVs2Part", "2Part.dat", "ms");
#else
        std::cout << "Cvode is not enabled.\n";
#endif
    }
};
