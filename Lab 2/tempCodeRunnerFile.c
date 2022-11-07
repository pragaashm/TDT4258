    if(n_blocks <= 2 && cache_org == sc){
        
        index = 0;
    }

    else{

        index = (access.address >> b_offset);
        index = (index << (b_offset + t_bits));
        index = (index >> (b_offset + t_bits));
    }