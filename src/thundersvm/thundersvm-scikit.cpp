//functions for scikit interface

#include <thundersvm/util/log.h>
#include <thundersvm/model/svc.h>
#include <thundersvm/model/svr.h>
#include <thundersvm/model/oneclass_svc.h>
#include <thundersvm/model/nusvc.h>
#include <thundersvm/model/nusvr.h>
#include <thundersvm/util/metric.h>
#include "thundersvm/cmdparser.h"

using std::fstream;
using std::stringstream;

extern "C" {
    SvmModel* sparse_model_scikit(int row_size, float* val, int* row_ptr, int* col_ptr, float* label,
                                  int svm_type, int kernel_type, int degree, float gamma, float coef0,
                                  float cost, float nu, float tol, int probability,
                                  int weight_size, int* weight_label, float* weight,
                                  int verbose, int max_iter,
                                  int* n_features, int* n_classes){
        if(verbose)
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Enabled, "false");
        else
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Enabled, "true");
        DataSet train_dataset;
        train_dataset.load_from_sparse(row_size, val, row_ptr, col_ptr, label);
        SvmModel* model;
        switch (svm_type){
            case SvmParam::C_SVC:
                model = new SVC();
                break;
            case SvmParam::NU_SVC:
                model = new NuSVC();
                break;
            case SvmParam::ONE_CLASS:
                model = new OneClassSVC();
                break;
            case SvmParam::EPSILON_SVR:
                model = new SVR();
                break;
            case SvmParam::NU_SVR:
                model = new NuSVR();
                break;
        }
        model->set_max_iter(max_iter);

        //todo add this to check_parameter method
        if (svm_type == SvmParam::NU_SVC) {
            train_dataset.group_classes();
            for (int i = 0; i < train_dataset.n_classes(); ++i) {
                int n1 = train_dataset.count()[i];
                for (int j = i + 1; j < train_dataset.n_classes(); ++j) {
                    int n2 = train_dataset.count()[j];
                    if (nu * (n1 + n2) / 2 > min(n1, n2)) {
                        printf("specified nu is infeasible\n");
                        return model;
                    }
                }
            }
        }
        SvmParam param_cmd;
        param_cmd.weight_label = NULL;
        param_cmd.weight = NULL;
        param_cmd.svm_type = static_cast<SvmParam::SVM_TYPE> (svm_type);
        param_cmd.kernel_type = static_cast<SvmParam::KERNEL_TYPE> (kernel_type);
        param_cmd.degree = degree;
        param_cmd.gamma = (float_type)gamma;
        param_cmd.coef0 = (float_type)coef0;
        param_cmd.C = (float_type)cost;
        param_cmd.nu = (float_type)nu;
        param_cmd.epsilon = (float_type)tol;
        param_cmd.probability = probability;
        if(weight_size != 0) {
            param_cmd.nr_weight = weight_size;
            param_cmd.weight = (float_type *) malloc(weight_size * sizeof(float_type));
            param_cmd.weight_label = (int *) malloc(weight_size * sizeof(int));
            for (int i = 0; i < weight_size; i++) {
                param_cmd.weight[i] = weight[i];
                param_cmd.weight_label[i] = weight_label[i];
            }
        }

        model->train(train_dataset, param_cmd);
        LOG(INFO) << "training finished";
        n_features[0] = train_dataset.n_features();
        n_classes[0] = model->get_n_classes();
        return model;

    }

    int sparse_predict(int row_size, float* val, int* row_ptr, int* col_ptr, SvmModel *model, float* predict_label){
        DataSet predict_dataset;
        predict_dataset.load_from_sparse(row_size, val, row_ptr, col_ptr, (float *)NULL);
        vector<float_type> predict_y;
        predict_y = model->predict(predict_dataset.instances(), 10000);
        for (int i = 0; i < predict_y.size(); ++i) {
            predict_label[i] = predict_y[i];
        }
        return 0;
    }

    SvmModel* dense_model_scikit(int row_size, int features, float* data, float* label,
                                 int svm_type, int kernel_type, int degree, float gamma, float coef0,
                                 float cost, float nu, float tol, int probability,
                                 int weight_size, int* weight_label, float* weight,
                                 int verbose, int max_iter,
                                 int* n_features, int* n_classes){
        if(verbose)
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Enabled, "false");
        else
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Enabled, "true");
        DataSet train_dataset;
        train_dataset.load_from_dense(row_size, features, data, label);
        SvmModel* model;
        switch (svm_type){
            case SvmParam::C_SVC:
                model = new SVC();
                break;
            case SvmParam::NU_SVC:
                model = new NuSVC();
                break;
            case SvmParam::ONE_CLASS:
                model = new OneClassSVC();
                break;
            case SvmParam::EPSILON_SVR:
                model = new SVR();
                break;
            case SvmParam::NU_SVR:
                model = new NuSVR();
                break;
        }

        model->set_max_iter(max_iter);
        //todo add this to check_parameter method
        if (svm_type == SvmParam::NU_SVC) {
            train_dataset.group_classes();
            for (int i = 0; i < train_dataset.n_classes(); ++i) {
                int n1 = train_dataset.count()[i];
                for (int j = i + 1; j < train_dataset.n_classes(); ++j) {
                    int n2 = train_dataset.count()[j];
                    if (nu * (n1 + n2) / 2 > min(n1, n2)) {
                        printf("specified nu is infeasible\n");
                        return model;
                    }
                }
            }
        }


        SvmParam param_cmd;
        param_cmd.weight_label = NULL;
        param_cmd.weight = NULL;
        param_cmd.svm_type = static_cast<SvmParam::SVM_TYPE> (svm_type);
        param_cmd.kernel_type = static_cast<SvmParam::KERNEL_TYPE> (kernel_type);
        param_cmd.degree = degree;
        param_cmd.gamma = (float_type)gamma;
        param_cmd.coef0 = (float_type)coef0;
        param_cmd.C = (float_type)cost;
        param_cmd.nu = (float_type)nu;
        param_cmd.epsilon = (float_type)tol;
        param_cmd.probability = probability;
        if(weight_size != 0) {
            param_cmd.nr_weight = weight_size;
            param_cmd.weight = (float_type *) malloc(weight_size * sizeof(float_type));
            param_cmd.weight_label = (int *) malloc(weight_size * sizeof(int));
            for (int i = 0; i < weight_size; i++) {
                param_cmd.weight[i] = weight[i];
                param_cmd.weight_label[i] = weight_label[i];
            }
        }

        model->train(train_dataset, param_cmd);
        LOG(INFO) << "training finished";
        n_features[0] = train_dataset.n_features();
        n_classes[0] = model->get_n_classes();
        return model;

    }

    int dense_predict(int row_size, int features, float* data, SvmModel *model, float* predict_label){
        DataSet predict_dataset;
        predict_dataset.load_from_dense(row_size, features, data, (float*) NULL);
        vector<float_type> predict_y;
        predict_y = model->predict(predict_dataset.instances(), 10000);
        std::cout<<"predict_y:"<<predict_y[0]<<std::endl;
        std::cout<<"size"<<predict_y.size()<<std::endl;
        for (int i = 0; i < predict_y.size(); ++i) {
            predict_label[i] = predict_y[i];
        }
        std::cout<<"label[0]"<<predict_label[0]<<std::endl;
        return 0;
    }

    int n_sv(SvmModel* model){
        return model->total_sv();
    }


    void get_sv(int* row, int* col, float* data, int* data_size, SvmModel* model){
        DataSet::node2d svs = model->svs();
        row[0] = 0;
        int data_ind = 0;
        int col_ind = 0;
        int row_ind = 1;
        for(int i = 0; i < svs.size(); i++){
            row[row_ind] = row[row_ind - 1] + svs[i].size();
            row_ind++;
            for(int j = 0; j < svs[i].size(); j++){
                data[data_ind] = svs[i][j].value;
                data_ind++;
                col[col_ind] = svs[i][j].index;
                col_ind++;
            }
        }
        data_size[0] = data_ind;
        return ;
    }

    void get_support_classes(int* n_support, int n_class, SvmModel* model){
        SyncArray<int> n_sv(n_class);
        n_sv.copy_from(model->get_n_sv());
        int* n_sv_ptr = n_sv.host_data();
        for(int i = 0; i < n_sv.size(); i++){
            n_support[i] = n_sv_ptr[i];
        }
    }

    void get_coef(float* dual_coef, int n_class, int n_sv, SvmModel* model){
        SyncArray<float_type > coef((n_class - 1 ) * n_sv);
        coef.copy_from(model->get_coef());
        float_type * coef_ptr = coef.host_data();
        for(int i = 0; i < coef.size(); i++){
            dual_coef[i] = coef_ptr[i];
        }
    }

    void get_rho(float* rho_, int rho_size, SvmModel* model){
        SyncArray<float_type > rho(rho_size);
        rho.copy_from(model->get_rho());
        float_type * rho_ptr = rho.host_data();
        for(int i = 0; i < rho.size(); i++){
            rho_[i] = rho_ptr[i];
        }
    }

}
