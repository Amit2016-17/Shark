/*!
 * 
 *
 * \brief       Illustration of the OpenML component.
 * 
 * 
 *
 * \author      T. Glasmachers
 * \date        2016
 *
 *
 * \par Copyright 1995-2017 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://shark-ml.org/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//###begin<includes>
#include <shark/OpenML/OpenML.h>
#include <shark/Models/Kernels/GaussianRbfKernel.h>
#include <shark/Algorithms/Trainers/McSvmOVATrainer.h>
#include <iostream>

using namespace shark;
using namespace std;
//###end<includes>


int main(int argc, char** argv)
{
	try
	{
//###begin<key>
		// The following line sets the OpenML api key to the Shark library's
		// demo account. This account is for tutorial demonstration only.
		// It is a read-only key, which does not allow to make changes to
		// the OpenML data base.
		// NOTE: Always use your own api key (attached to your OpenML account)
		// for actual experiments. Otherwise your results will be lost and you
		// cannot receive credit for your work. Creating new flows and runs
		// with this key will silently fail.
		string api_key = "0076c004519625ecc7ad51e553f40dff";

		// register the api key in the global openML::connection object
		openML::connection.setKey(api_key);
//###end<key>

//###begin<query>
		// find a data set
//		openML::QueryResult result = openML::supervisedClassificationTasks();
//		result = openML::filter(result, "NumberOfInstances >= 100, NumberOfInstances <= 200, NumberOfFeatures <= 10, NumberOfMissingValues == 0");
//		openML::IDType datasetID = result[0].id;
//		// TODO: obtain supervised classification task from data set
		openML::IDType taskID = 11;   // this should be the result of a query at some point in the future
//###end<query>

//###begin<task>
		// instantiate the chosen task
		shared_ptr<openML::Task> task = openML::Task::get(taskID);
		task->print();
//###end<task>

//###begin<dataset>
		// obtain the data set underlying the task
		shared_ptr<openML::Dataset> dataset = task->dataset();
		dataset->print();
//###end<dataset>

//###begin<tagging>
		// set a tag
//		dataset->tag("shark-tutorial-demo-tag");
//###end<tagging>

//###begin<setup>
		// setup a learning machine to solve the task
		double C = 1.0;
		double gamma = 1.0;
		bool bias = false;
		GaussianRbfKernel<RealVector> kernel(gamma);
		McSvmOVATrainer<RealVector> trainer(&kernel, C, bias);
//###end<setup>

//###begin<flow>
		// define a flow representing the setup
		std::string flowName = trainer.name() + "." + kernel.name();
		std::vector<openML::Hyperparameter> params;
		params.push_back(openML::Hyperparameter("C", "regularization parameter, must be positive", "double"));
		params.push_back(openML::Hyperparameter("gamma", "kernel bandwidth parameter, must be positive", "double"));
		params.push_back(openML::Hyperparameter("bias", "presence or absence of the bias 'b' in the model", "bool"));
		shared_ptr<openML::Flow> flow = openML::Flow::create(flowName, "one-versus-all C-SVM with Gaussian RBF kernel", params);
		flow->print();
//###end<flows>

//###begin<run>
		// create a run object representing the results
		openML::Run run(task, flow);
//###end<run>

//###begin<hyperparam>
		run.setHyperparameterValue("C", trainer.C());                // ideally this would be automated
		run.setHyperparameterValue("gamma", kernel.gamma());         // ideally this would be automated
		run.setHyperparameterValue("bias", trainer.trainOffset());   // ideally this would be automated
		run.print();
//###end<hyperparam>

//###begin<execute>
		// execute the learning machine and fill the run with predictions
		cout << "training and predicting " << flush;
		ClassificationDataset data;
		task->loadData(data);
		Data<unsigned int> predictions;
		for (std::size_t r=0; r<task->repetitions(); r++)
		{
			CVFolds< LabeledData<RealVector, unsigned int> > folds = task->split(r, data);
			for (std::size_t f=0; f<task->folds(); f++)
			{
				ClassificationDataset traindata      = folds.training(f);
				ClassificationDataset validationdata = folds.validation(f);
				KernelClassifier<RealVector> model;
				trainer.train(model, traindata);
				predictions = model(validationdata.inputs());
				run.setPredictions(r, f, predictions);
				cout << "." << flush;
			}
		}
		cout << " done." << endl;
//###end<execute>

//###begin<commit>
		// upload the results to OpenML
		cout << "\nNOTE: the following call to commit() fails due to the read-only account.\n\n";
		run.commit();
		cout << "ID of the new run: " << run.id() << endl;
//###end<commit>
	}
	catch (std::exception const& ex)
	{
		cout << ex.what() << std::endl;
	}
}
