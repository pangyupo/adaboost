/*-----------------------------------------------------------------------------
 *  Author:			yuanyang
 *  Date:			20141225
 *  Description:	binary tree
 *-----------------------------------------------------------------------------*/

#ifndef BINARYTREE_HPP
#define BINARYTREE_HPP
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

/*  parameters for one tree */
struct tree_para
{
	int		nBins;		/* maximum number of quantization bins, <=256 is enough*/
	int		maxDepth;   /* max depth of the tree */
	double	minWeight;	/* minimum sample weight allow split*/
	double	fracFtrs;	/* fraction of features to sample for each node split */
	int		nThreads;	/* max number of computational threads to use */

	tree_para()
	{
		nBins = 256;
		maxDepth = 2;
		minWeight = 0.01;
		fracFtrs = 0.0625;
		nThreads = 8;
	}
};


/*  struct of the binary tree , save and load */
struct biTree
{
	Mat fids;		/* 1xK 32S feature index for each node , K for number of nodes*/
	Mat thrs;		/* 1xK 64F thresholds for each node */
	Mat child;		/* 1xK 32S child index for each node */
	Mat hs;			/* 1xK 64F log ratio (.5*log(p/(1-p)) at each node, used later to decide polarity */
	Mat weights;	/* 1xK 64F total sample weight at each node */
	Mat depth;		/* 1xK 32S depth of node*/
};


/*  struct of training data and other informations */
struct data_pack
{
	Mat neg_data;			/* negative training data,				featuredim x numbers0 */
	Mat pos_data;			/* positive training data,				featuredim x numbers1*/
	Mat wts0;				/* weights for negative data			numbers0   x 1*/
	Mat wts1;				/* weights for positive data			numbers1   x 1 */
	Mat Xmin;				/* minimun value of each dimension		featuredim x 1*/
	Mat Xmax;				/* maximun value of each dimension		featuredim x 1*/
	Mat Xstep;				/* quantization step					featuredim x 1 */
};


class binaryTree
{
	
	public:

		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  SetDebug
		 *  Description:  output debug information or not
		 * =====================================================================================
		 */
		void SetDebug( bool isDebug );		/* in: wanna debug information */
		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  Train
		 *  Description:  train the binary tree
		 *			out:  true->no error
		 *	    warning:  this will change the original data in train_data ( quantization is expensive
         *	              save the quantized data back will save the computation)!!
		 * =====================================================================================
		 */
		bool Train( 
				data_pack & train_data,			/* input&output : training data and weights info */
			    const tree_para &paras			/* input		: tree paras */
			   );


		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  Apply
		 *  Description:  predict giving data
		 *		    out:  true->no error
		 * =====================================================================================
		 */
		bool Apply( const Mat &inputData,			/* inp  featuredim x number_of_sample, column vector*/
					  Mat &predictResult) const;	/* out  predicted label number_of_sample x 1, column vector*/		

		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  showTreeInfo
		 *  Description:  output the information about the tree
		 * =====================================================================================
		 */
		void showTreeInfo() const;

		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  getTrainError
		 *  Description:  return the weighted train error
		 * =====================================================================================
		 */
		double getTrainError() const;


		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  scaleHs
		 *  Description:  scale the hs vector, used in adaboosting training
		 * =====================================================================================
		 */
		void scaleHs( double factor );					/*  in : scale factor */


		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  getTree
		 *  Description:  return the tree
		 * =====================================================================================
		 */
		const biTree* getTree() const;


		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  setTreeModel
		 *  Description:  set the m_tree
		 * =====================================================================================
		 */
		bool setTreeModel( const biTree& model );		/*  in : model */
	private:

		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  checkTreeParas
		 *  Description:  check if the parameters is right
		 *			out:  true if the parameter is in the right range
		 * =====================================================================================
		 */
		bool checkTreeParas( const tree_para & p ) const;	/* input parameter */

		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  computeXMinMax
		 *  Description:  like matlab's XMin = min(min(X0), min(X1)) - 0.01
		 *								XMax = max(max(X0), max(X1)) + 0.01
		 *			in : X0, X1
		 *			out: XMin XMax
		 * =====================================================================================
		 */
		void computeXMinMax( const Mat &X0,		/* neg data */
							 const Mat &X1,		/* pos data , column feature */
							 Mat& XMin,			
							 Mat& XMax) const;


		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  binaryTreeTrain
		 *  Description:  train the tree
		 *			out:  errors_st			error when using the selected feature  number_of_feature_selected x 1
		 *				  featureSelected	selected feature					   number_of_feature_selected x 1
		 * =====================================================================================
		 */
		bool binaryTreeTrain(   const Mat &neg_data,			// in column feature featuredim x number
								const Mat &pos_data,			// in same as neg_data
								const Mat &norm_neg_weight,		// in sample weight, normalized( sum(neg) + sum(pos) = 1)
								const Mat &norm_pos_weight,		// in 
								int nBins,						// in number of bins
								double prior,					// in prior of the error rate
								const Mat &fids_st,				// in index of the selected feature
								int nthreads,					// in numbers of the threads use in training
								Mat &errors_st,					// out
								Mat &thresholds);				// out


		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  computeCDF
		 *  Description:  compute the cdf, give the data and weights
		 * =====================================================================================
		 */
		bool computeCDF(	const Mat & sampleData,				// in samples	featuredim x numberOfSamples
							const Mat & weights,				// in weights	numberOfSamples x 1
							int nBins,							// in number of bins
							vector<double> &cdf					// out cdf
						) const;

		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  any
		 *  Description:  true if there's at least one non-zero-element 
		 * =====================================================================================
		 */
		bool any( const Mat& input) const;								// in 


		/* 
		 * ===  FUNCTION  ======================================================================
		 *         Name:  convertHs
		 *  Description:  convert hs to label( 1 or -1), it is the output of the tree~
		 *					hs = (hs>0)*2-1;
		 * =====================================================================================
		 */
		void convertHsToDouble();

private:
		biTree m_tree;						/*  model struct */
		bool m_debug;						/* want output? */
		double m_error;						/* training error, used for adaboost training only */

};
#endif
