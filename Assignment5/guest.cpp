void *guest_thread(void *args)
{
    int guest_index = -1;
    for (int i = 0; i < y; i++)
    {
        if ((pthread_t *)args == &guest_tid[i])
        {
            guest_index = i;
            break;
        }
    }
    assert(guest_index != -1);
    Guest *guest = &guests[guest_index];
    while (1)
    {
        sleep(get_random(10, 20));

        pthread_mutex_lock(&mutex_id);

        while (room_priority_queue.begin()->first >= guest->priority)
        {
            pthread_cond_wait(&cond_id, &mutex_id);
        }
        int room_no = room_priority_queue.begin()->second;

        rooms[room_no].number_of_guests++;

        if (room_priority_queue.begin()->first != 0)
        {
            // intterupt signal
            Guest *guest_ = rooms[room_no].guest;
            sem_post(&sem_id[guest_->id]);
            out<<"Removing the Guest: "<<guest_->id<<" with priority "<<room_priority_queue.begin()->first<<" from Room "<<room_no<<" and "<<"Adding the guest: "<<guest_index<<" with priority "<<guest->priority<<endl;
        }else{
            out<<"Adding the Guest: "<<guest_index<<" with priority "<<guest->priority<<" to the empty room "<<room_no<<endl;
        }

        room_priority_queue.erase(room_priority_queue.begin());
        if (rooms[room_no].number_of_guests < 2)
        {
            room_priority_queue.insert(make_pair(guest->priority, room_no));
        }
        else
        {
            room_priority_queue.insert(make_pair(y + 1, room_no));
        }

        rooms[room_no].guest = guest;

        int random_number = get_random(10, 30);
        struct timespec sleep_time;
        clock_gettime(CLOCK_REALTIME, &sleep_time);
        sleep_time.tv_sec += random_number;

        pthread_mutex_unlock(&mutex_id);
        pthread_cond_broadcast(&cond_id);

        int res = -1;
        res = sem_timedwait(&sem_id[guest->id], &sleep_time);

        pthread_mutex_lock(&mutex_id);
        if (res == 0)
        {
            // interrupt
            // do nothing
        }
        else
        {
            // not interrupt
            out<< "Guest: "<<guest_index<<" with priority: "<<guest->priority<<" left the room: "<<room_no<<" after compeleting its stay"<<endl;
            if (rooms[room_no].number_of_guests < 2)
            {
                room_priority_queue.erase(room_priority_queue.find(make_pair(guest->priority, room_no)));
                room_priority_queue.insert(make_pair(0, room_no));
            }
            else
            {
                uncleaned_rooms++;
                rooms[room_no].cleaned = false;
                if (uncleaned_rooms == n){
                    for(int i=0;i<n;i++){
                        sem_post(&sem_clean_wait);
                    }
                }
            }
            rooms[room_no].guest = NULL;
            rooms[room_no].previous_guest_time += random_number;
        }

        pthread_mutex_unlock(&mutex_id);
        pthread_cond_broadcast(&cond_id);
    }
    pthread_exit(0);
}