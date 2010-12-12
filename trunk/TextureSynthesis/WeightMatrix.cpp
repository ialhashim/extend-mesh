#include "WeightMatrix.h"

WeightMatrix::WeightMatrix(BlockType type)
{
	this->m = MatrixXf::Zero(BlockSize, BlockSize);

	switch (type)
	{
        case NONE_BLOCK:
                break;

	case VERTICAL:
		for(int i = 0; i < BandSize; i++)
			m.col(i) = VectorXf::Constant(BlockSize, 1.0f / (float)(BandSize - i));
		break;

        case HORIZONTAL:
                break;

	case V_BOTHSIDES:
		for(int i = 0; i < BandSize; i++)
		{
			m.col(i) = VectorXf::Constant(BlockSize, 1.0f / (BandSize - i));
			m.col((BlockSize - 1) - i) = VectorXf::Constant(BlockSize, 1.0f / (BandSize - i));
		}
		break;

	case L_SHAPED:
		for(int i = 0; i < BandSize; i++){
			m.block(i, i, 1, BlockSize - i) =  MatrixXf::Constant(1, BlockSize - i, 1.0f / (float)(BandSize - i));
			m.block(i, i, BlockSize - i, 1) =  MatrixXf::Constant(BlockSize - i, 1, 1.0f / (float)(BandSize - i));
		}
		break;

	case N_SHAPED:
		for(int i = 0; i < BandSize; i++){
			m.block(i, i, 1, BlockSize - (i*2)) =  MatrixXf::Constant(1, BlockSize - (i*2), 1.0f / (BandSize - i));
			m.block(i, i, BlockSize - i, 1) =  MatrixXf::Constant(BlockSize - i, 1, 1.0f / (BandSize - i));
			m.block(i, (BlockSize - 1) - i, BlockSize - i, 1) =  MatrixXf::Constant(BlockSize - i, 1, 1.0f / (BandSize - i));
		}
                break;
	}
}
