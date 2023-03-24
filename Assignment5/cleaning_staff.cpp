void *cleaning_staff_thread(void *args)
{
    while (1)
    {
        // wakes up when required
        sem_wait(&sem_clean_wait);

        // select room
        sem_wait(&sem_cleaner);
        vector<int> idx;
        for (int i = 0; i < n; i++)
            if (!rooms[i].cleaned)
                idx.push_back(i);

        int select_idx = idx[get_random(0, idx.size() - 1)];
        Room *selected_room = &rooms[select_idx];
        selected_room->cleaned = true;
        out<<"Cleaning Room No. "<<select_idx<<endl;
        sem_post(&sem_cleaner);

        // do cleaning
        sleep(selected_room->previous_guest_time);
        selected_room->previous_guest_time = 0;
        selected_room->guest = NULL;
        selected_room->number_of_guests = 0;

        sem_wait(&sem_cleaner);
        uncleaned_rooms--;
        if(uncleaned_rooms == 0){
            auto itr = room_priority_queue.end();
            itr--;
            while (itr->first == y+1)
            {
                auto temp = *itr;
                room_priority_queue.erase(itr);
                room_priority_queue.insert({0,temp.second});
                itr = room_priority_queue.end();
                itr--;
            }     
            pthread_cond_broadcast(&cond_id);  
        }
        sem_post(&sem_cleaner);
    }

    pthread_exit(0);
}