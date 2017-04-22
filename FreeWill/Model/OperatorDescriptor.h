#ifndef OPERATORDESCRIPTOR_H
#define OPERATORDESCRIPTOR_H

#include "../Operator/Operator.h"
#include "../Operator/Activation.h"
#include "../Operator/ActivationDerivative.h"
#include "../Operator/Convolution.h"
#include "../Operator/ConvolutionDerivative.h"
#include "../Operator/CrossEntropyLoss.h"
#include "../Operator/DotProductWithBias.h"
#include "../Operator/DotProductWithBiasDerivative.h"
#include "../Operator/ElementwiseAdd.h"
#include "../Operator/MaxPooling.h"
#include "../Operator/MaxPoolingDerivative.h"
#include "../Operator/SigmoidCrossEntropyLossDerivative.h"
#include "../Operator/SoftmaxLogLoss.h"
#include "../Operator/SoftmaxLogLossDerivative.h"
#include "TensorDescriptor.h"
#include <any>
#include <fstream>

namespace FreeWill
{
    //xxx should operator has datatype?

    class Model;
    class Solver;

    typedef std::string OperatorDescriptorHandle;

    class OperatorDescriptor
    {
        friend class Model;
        friend class Solver;

        constexpr static const float topBottomMargin = 20;
        constexpr static const float centerSpace = 40;
        constexpr static const float anchorSpace = 5;
        constexpr static const float anchorHeight = 15;
        constexpr static const float anchorWidth = 80;
        constexpr static const float leftRightMargin = 5;
        constexpr static const float topSpace = 32;
        constexpr static const float bottomSpace = 32;
        constexpr static const float leftSpace = 5;
        constexpr static const float rightSpace = 5;

    private:
        std::string m_name;
        DataType m_dataType;
        OperatorName m_operatorName;
        std::map<std::string, FreeWill::TensorDescriptorHandle> m_inputs;
        std::map<std::string, FreeWill::TensorDescriptorHandle> m_outputs;
        std::map<std::string, std::any> m_parameters;
        std::map<DeviceType, std::vector<std::variant<Operator<DeviceType::GPU_CUDA>*, Operator<DeviceType::CPU_NAIVE>*>>> m_operators;

        OperatorDescriptor(const std::string &name, OperatorName operatorName,
                           const std::map<std::string, FreeWill::TensorDescriptorHandle> &inputs,
                           const std::map<std::string, FreeWill::TensorDescriptorHandle> &outputs,
                           const std::map<std::string, std::any> &parameters, DataType dataType = DataType::FLOAT);
        ~OperatorDescriptor();

        void generateSVGDiagram(std::ostream &outputStream, unsigned int &width, unsigned int &height);
        void evaluateSVGDiagramSize(unsigned int &width, unsigned int &height);

        template<DeviceType DeviceUsed>
        bool setInput(Operator<DeviceUsed> *operatorBase, const std::string &inputName, std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            if (m_inputs.find(inputName) == m_inputs.end())
            {
                return false;
            }

            TensorBase<DeviceUsed> *tensorBase = tensors[m_inputs[inputName].first]->getTensorForDevice<DeviceUsed>();

            if (m_inputs[inputName].second != Shape())
            {
                if (! tensorBase->reshape(m_inputs[inputName].second))
                {
                    std::cerr << "Reshape failed for input: " << inputName << " tensor: " << tensorBase->name() << " from: " << tensorBase->shape() << " to: " << m_inputs[inputName].second << std::endl;
                    return false;
                }
            }

            operatorBase->setInputParameter(inputName, tensorBase);

            return true;
        }

        template<DeviceType DeviceUsed>
        bool setOutput(Operator<DeviceUsed> *operatorBase, const std::string &outputName, std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            if (m_outputs.find(outputName) == m_outputs.end())
            {
                return false;
            }

            TensorBase<DeviceUsed> *tensorBase = tensors[m_outputs[outputName].first]->getTensorForDevice<DeviceUsed>();

            if (m_outputs[outputName].second != Shape())
            {
                if (! tensorBase->reshape(m_outputs[outputName].second))
                {
                    std::cerr << "Reshape failed for output: " << outputName << " tensor: " << tensorBase->name() << " from: " << tensorBase->shape() << " to: " << m_outputs[outputName].second << std::endl;
                    return false;
                }
            }

            operatorBase->setOutputParameter(outputName, tensorBase);

            return true;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initActivation(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;
            if (m_parameters.find("Mode") == m_parameters.end())
            {
                return nullptr;
            }

            FreeWill::ActivationMode mode = std::any_cast<FreeWill::ActivationMode>(m_parameters["Mode"]);
            switch(mode)
            {
            case ActivationMode::SIGMOID:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new Activation<ActivationMode::SIGMOID, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new Activation<ActivationMode::SIGMOID, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new Activation<SIGMOID, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;
                }
                break;
            case ActivationMode::RELU:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new Activation<ActivationMode::RELU, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new Activation<ActivationMode::RELU, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new Activation<RELU, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;

                }
                break;
            case ActivationMode::TANH:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new Activation<ActivationMode::TANH, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new Activation<ActivationMode::TANH, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new Activation<TANH, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;

                }
                break;
            case ActivationMode::CLIPPED_RELU:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new Activation<ActivationMode::CLIPPED_RELU, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new Activation<ActivationMode::CLIPPED_RELU, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new Activation<CLIPPED_RELU, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;

                }
                break;
            }

            if (!setInput(operatorBase, "Input", tensors) || !setOutput(operatorBase, "Output", tensors))
            {
                delete operatorBase;
                operatorBase = nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initActivationDerivative(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;
            if (m_parameters.find("Mode") == m_parameters.end())
            {
                return nullptr;
            }

            FreeWill::ActivationMode mode = std::any_cast<FreeWill::ActivationMode>(m_parameters["Mode"]);
            switch(mode)
            {
            case ActivationMode::SIGMOID:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new ActivationDerivative<ActivationMode::SIGMOID, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new ActivationDerivative<ActivationMode::SIGMOID, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new ActivationDerivative<SIGMOID, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;
                }
                break;
            case ActivationMode::RELU:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new ActivationDerivative<ActivationMode::RELU, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new ActivationDerivative<ActivationMode::RELU, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new ActivationDerivative<RELU, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;
                }
                break;
            case ActivationMode::TANH:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new ActivationDerivative<ActivationMode::TANH, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new ActivationDerivative<ActivationMode::TANH, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new ActivationDerivative<TANH, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;
                }
                break;
            case ActivationMode::CLIPPED_RELU:
                switch(m_dataType)
                {
                case DataType::FLOAT:
                    operatorBase = new ActivationDerivative<ActivationMode::CLIPPED_RELU, DeviceUsed, float>();
                    break;
                case DataType::DOUBLE:
                    operatorBase = new ActivationDerivative<ActivationMode::CLIPPED_RELU, DeviceUsed, double>();
                    break;
                /*case UNSIGNED_INT:
                    operatorBase = new ActivationDerivative<CLIPPED_RELU, DeviceUsed, unsigned int>();
                    break;*/
                case DataType::UNSIGNED_INT:
                    return nullptr;
                }
                break;
            }
            if (!setInput(operatorBase, "Output", tensors) ||
                    !setInput(operatorBase, "OutputDelta", tensors) ||
                    !setOutput(operatorBase, "InputDelta", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initConvolution(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new Convolution<DeviceUsed, float>();

                break;
            case DataType::DOUBLE:
                operatorBase = new Convolution<DeviceUsed, double>();

                break;
            /*case UNSIGNED_INT:
                operatorBase = new Convolution<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;

            }

            if (!setInput(operatorBase, "Input", tensors) ||
                    !setInput(operatorBase, "FeatureMap", tensors) ||
                    !setInput(operatorBase, "Bias", tensors) ||
                    !setOutput(operatorBase, "Output", tensors))
            {
                delete operatorBase;
                return nullptr;
            }


            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initConvolutionDerivative(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new ConvolutionDerivative<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new ConvolutionDerivative<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new ConvolutionDerivative<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;

            }

            if (!setInput(operatorBase, "PrevActivation", tensors) ||
                    !setInput(operatorBase, "FeatureMap", tensors) ||
                    !setInput(operatorBase, "OutputGrad", tensors) ||
                    !setOutput(operatorBase, "FeatureMapGrad", tensors) ||
                    !setOutput(operatorBase, "BiasGrad", tensors) ||
                    !setOutput(operatorBase, "InputGrad", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initCrossEntropyLoss(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new CrossEntropyLoss<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new CrossEntropyLoss<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new CrossEntropyLoss<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "Input", tensors) ||
                    !setInput(operatorBase, "Label", tensors) ||
                    !setOutput(operatorBase, "Cost", tensors))
            {
                delete operatorBase;
                return nullptr;
            }


            return operatorBase;
        }


        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initDotProductWithBias(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new DotProductWithBias<DeviceUsed, float>();

                break;
            case DataType::DOUBLE:
                operatorBase = new DotProductWithBias<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new DotProductWithBias<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "Input", tensors) ||
                    !setInput(operatorBase, "Weight", tensors) ||
                    !setInput(operatorBase, "Bias", tensors) ||
                    !setOutput(operatorBase, "Output", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;

        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initDotProductWithBiasDerivative(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new DotProductWithBiasDerivative<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new DotProductWithBiasDerivative<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new DotProductWithBiasDerivative<DeviceUsed, unsigned int>();

                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "InputActivation", tensors) ||
                    !setInput(operatorBase, "OutputDelta", tensors) ||
                    !setInput(operatorBase, "Weight", tensors) ||
                    !setOutput(operatorBase, "WeightGrad", tensors) ||
                    !setOutput(operatorBase, "BiasGrad", tensors) ||
                    !setOutput(operatorBase, "InputDelta", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initElementwiseAdd(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            float rate = 1.0;

            if (m_parameters.find("Rate") == m_parameters.end())
            {
                rate = std::any_cast<float>(m_parameters["Rate"]);
            }

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new ElementwiseAdd<DeviceUsed, float>(rate);
                break;
            case DataType::DOUBLE:
                operatorBase = new ElementwiseAdd<DeviceUsed, double>(rate);
                break;
            /*case UNSIGNED_INT:
                operatorBase = new ElementwiseAdd<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "OperandA", tensors) ||
                    !setInput(operatorBase, "OperandB", tensors) ||
                    !setOutput(operatorBase, "Result", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initMaxPooling(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new MaxPooling<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new MaxPooling<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new MaxPooling<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "Input", tensors) ||
                    !setOutput(operatorBase, "Output", tensors) ||
                    !setOutput(operatorBase, "SwitchX", tensors) ||
                    !setOutput(operatorBase, "SwitchY", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;

        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initMaxPoolingDerivative(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new MaxPoolingDerivative<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new MaxPoolingDerivative<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new MaxPoolingDerivative<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }


            if (!setInput(operatorBase, "Output", tensors) ||
                    !setInput(operatorBase, "OutputGrad", tensors) ||
                    !setInput(operatorBase, "Input", tensors) ||
                    !setInput(operatorBase, "SwitchX", tensors) ||
                    !setInput(operatorBase, "SwitchY", tensors) ||
                    !setOutput(operatorBase, "InputGrad", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initSigmoidCrossEntropyLossDerivative(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new SigmoidCrossEntropyLossDerivative<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new SigmoidCrossEntropyLossDerivative<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new SigmoidCrossEntropyLossDerivative<DeviceUsed, unsigned int>();

                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "Input", tensors) ||
                    !setInput(operatorBase, "Label", tensors) ||
                    !setOutput(operatorBase, "Output", tensors))
            {
                delete operatorBase;
                return nullptr;
            }


            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initSoftmaxLogLoss(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch (m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new SoftmaxLogLoss<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new SoftmaxLogLoss<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new SoftmaxLogLoss<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "Input", tensors) ||
                    !setInput(operatorBase, "Label", tensors) ||
                    !setOutput(operatorBase, "Cost", tensors) ||
                    !setOutput(operatorBase, "Output", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        Operator<DeviceUsed> *initSoftmaxLogLossDerivative(std::map<std::string, FreeWill::TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_dataType)
            {
            case DataType::FLOAT:
                operatorBase = new SoftmaxLogLossDerivative<DeviceUsed, float>();
                break;
            case DataType::DOUBLE:
                operatorBase = new SoftmaxLogLossDerivative<DeviceUsed, double>();
                break;
            /*case UNSIGNED_INT:
                operatorBase = new SoftmaxLogLossDerivative<DeviceUsed, unsigned int>();
                break;*/
            case DataType::UNSIGNED_INT:
                return nullptr;
            }

            if (!setInput(operatorBase, "Output", tensors) ||
                    !setInput(operatorBase, "Label", tensors) ||
                    !setOutput(operatorBase, "InputGrad", tensors))
            {
                delete operatorBase;
                return nullptr;
            }

            return operatorBase;
        }

        template<DeviceType DeviceUsed>
        void evaluate()
        {
            auto iter = m_operators[DeviceUsed].begin();

            for(;iter != m_operators[DeviceUsed].end(); ++iter)
            {
                std::get<Operator<DeviceUsed>*>(*iter)->evaluate();
            }
        }

        template<DeviceType DeviceUsed>
        void evaluateWithParameterUpdate(const std::map<std::string, std::any> &newParameters)
        {
            auto iter = m_operators[DeviceUsed].begin();

            for(;iter != m_operators[DeviceUsed].end(); ++iter)
            {
                Operator<DeviceUsed> *operatorBase = std::get<Operator<DeviceUsed>*>(*iter);
                switch(m_operatorName)
                {
                case FreeWill::OperatorName::ACTIVATION:
                case FreeWill::OperatorName::ACTIVATION_DERIVATIVE:
                case FreeWill::OperatorName::CONVOLUTION:
                case FreeWill::OperatorName::CONVOLUTION_DERIVATIVE:
                case FreeWill::OperatorName::CROSS_ENTROPY_LOSS:
                case FreeWill::OperatorName::DOT_PRODUCT_WITH_BIAS:
                case FreeWill::OperatorName::DOT_PRODUCT_WITH_BIAS_DERIVATIVE:
                case FreeWill::OperatorName::MAX_POOLING:
                case FreeWill::OperatorName::MAX_POOLING_DERIVATIVE:
                case FreeWill::OperatorName::SIGMOID_CROSS_ENTROPY_LOSS_DERIVATIVE:
                case FreeWill::OperatorName::SOFTMAX_LOG_LOSS:
                case FreeWill::OperatorName::SOFTMAX_LOG_LOSS_DERIVATIVE:
                    break;
                case FreeWill::OperatorName::ELEMENTWISE_ADD:
                    if (newParameters.find("Rate") != newParameters.end())
                    {
                        float rate = std::any_cast<float>(newParameters.at("Rate"));
                        switch(m_dataType)
                        {
                        case DataType::FLOAT:
                            dynamic_cast<ElementwiseAdd<DeviceUsed, float>*>(operatorBase)->setRate(rate);
                            break;
                        case DataType::DOUBLE:
                            dynamic_cast<ElementwiseAdd<DeviceUsed, double>*>(operatorBase)->setRate(rate);
                            break;
                        case DataType::UNSIGNED_INT:
                            break;
                        }
                    }
                    break;
                }

                operatorBase->evaluate();
            }
        }

        template<DeviceType DeviceUsed = DeviceType::CPU_NAIVE>
        bool init(std::map<std::string, TensorDescriptor*> &tensors)
        {
            Operator<DeviceUsed> *operatorBase = nullptr;

            switch(m_operatorName)
            {
            case OperatorName::ACTIVATION:
                operatorBase = initActivation<DeviceUsed>(tensors);
            break;
            case OperatorName::ACTIVATION_DERIVATIVE:
                operatorBase = initActivationDerivative<DeviceUsed>(tensors);
            break;
            case OperatorName::CONVOLUTION:
                operatorBase = initConvolution<DeviceUsed>(tensors);
            break;
            case OperatorName::CONVOLUTION_DERIVATIVE:
                operatorBase = initConvolutionDerivative<DeviceUsed>(tensors);
            break;
            case OperatorName::CROSS_ENTROPY_LOSS:
                operatorBase = initCrossEntropyLoss<DeviceUsed>(tensors);
            break;
            case OperatorName::DOT_PRODUCT_WITH_BIAS:
                operatorBase = initDotProductWithBias<DeviceUsed>(tensors);
            break;
            case OperatorName::DOT_PRODUCT_WITH_BIAS_DERIVATIVE:
                operatorBase = initDotProductWithBiasDerivative<DeviceUsed>(tensors);
            break;
            case OperatorName::ELEMENTWISE_ADD:
                operatorBase = initElementwiseAdd<DeviceUsed>(tensors);
            break;
            case OperatorName::MAX_POOLING:
                operatorBase = initMaxPooling<DeviceUsed>(tensors);
            break;
            case OperatorName::MAX_POOLING_DERIVATIVE:
                operatorBase = initMaxPoolingDerivative<DeviceUsed>(tensors);
            break;
            case OperatorName::SIGMOID_CROSS_ENTROPY_LOSS_DERIVATIVE:
                operatorBase = initSigmoidCrossEntropyLossDerivative<DeviceUsed>(tensors);
            break;
            case OperatorName::SOFTMAX_LOG_LOSS:
                operatorBase = initSoftmaxLogLoss<DeviceUsed>(tensors);
            break;
            case OperatorName::SOFTMAX_LOG_LOSS_DERIVATIVE:
                operatorBase = initSoftmaxLogLossDerivative<DeviceUsed>(tensors);
            break;
            }

            if (!operatorBase)
            {
                return false;
            }

            if (!operatorBase->init())
            {
                delete operatorBase;
                return false;
            }

            m_operators[DeviceUsed].push_back(operatorBase);

            return true;
        }
    };
}

#endif