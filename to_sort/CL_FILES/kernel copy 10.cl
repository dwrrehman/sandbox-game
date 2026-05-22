

// 202307182.201201:

/// we should do a optimization     to where we don't put *n in the array anymore! (at least physically speaking!)   and we allow the array to be resized/reallocated dynamically so that each z value consumes the minimal amount of memory that it actually needs to continue running the graph. 


	//  we are going to do the "exponentially decreasing in frequency" opt to the resizing of the array, so its not a a bottle neck. so yeah. 


	//   we also have to figure out the way we are going to output the z value list--      

//			the candidate list needs to be dynamically resized/reallocated as well!!! so yeah. 











__kernel void execute_z_value(__global unsigned int* input, __global unsigned int* output) {

	int global_id = get_global_id(0);

	output[global_id] = input[global_id] * input[global_id];

	

	// ...

}






/*


void reduce(vector out, nat s, nat radix, nat length) {
    for (nat i = 0, p = 1; i < length; i++, p *= radix)
        out[i] = (s / p) % radix;
}
*/















/*       this is the code that turns a get_global_id   value into a     ("m1_array") m_1 hole fillings array       

			which        we turn that m1_array into a graph,        which the current nfgp does, right now. 

			


	//   turns a z value into an array of options.  (of size hole count, n,      with each hole having m possible values. 
													)



	

void reduce(vector out, nat s, nat radix, nat length) {
    for (nat i = 0, p = 1; i < length; i++, p *= radix)
        out[i] = (s / p) % radix;
}




	202307182.165841:


			so actuallyyy in order to generate options for all holes in graphs for 1space or 2space,  we actually need to 

			have the reduce function generate the output array, with two different radix's...   and use one, for the 0...k-1 elements, 
						and use another radix for the k...holecount    elements. 

				ie, we will have to make a more custom built reduce opperation to turn the number that we are given into an actual graph, 

					at least we will have to do all of this special logic for 1sp and 2sp. 

							for 0sp, we are off the hook, and can just use the vanilla reduce operation function. 
							ie, the code here:


								


									void reduce(vector out, nat s, nat radix, nat length) {
									    for (nat i = 0, p = 1; i < length; i++, p *= radix)
									        out[i] = (s / p) % radix;
									}




	
					so yeah, shouldnt be too bad. 


				to start with, i ithink i we are going to 

	
					make the opencl program generate all z values in zero space, and not prune anything, just output all of them, 
					and same for the nfgp version, and compare the results. 

						and then, we will start adding pruning metrics to both,

							(or rather, enabling pruning metrics, for nfgp, because we are going to add them in now!)




						



			shouldnt be too bad!



	yay


*/







