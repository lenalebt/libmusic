#ifndef PROGRESS_CALLBACK_HPP
#define PROGRESS_CALLBACK_HPP

#include <string>
#include <iostream>
#include <iomanip>

namespace music
{
    /**
     * @brief This class is a progress callback functor.
     * 
     * This class is used to signal progress to the calling part of a function,
     * even if the function is not finished yet. This can be used to inform the user
     * of progress, or similar things. The function needs to support this - 
     * functions that are known to have a looong runtime should support this.
     * 
     * @author Lena Brueder
     * @date 2012-08-13
     */
    class ProgressCallback
    {
    private:
        
    protected:
        
    public:
        /**
         * @brief This function will be called when some progress is made.
         * 
         * @param id the ID of the function that makes progress.
         * @param percent the amount of the job that is finished (0.0 <= percent <= 1.0), if applicable.
         *      if not applicable, percent < 0.0.
         * @param progressMessage an optional message that might be presented to the user.
         */
        virtual void progress(const std::string& id, double percent, const std::string& progressMessage)=0;
    };

    /**
     * @brief This class is a progress callback functor that encapsulates the id.
     * 
     * This class adds its ID to the call of progress() from ProgressCallback. Usage:
     * @code
     * //assume that this object is a ProgressCallback object.
     * someOtherObject.doSomething(ProgressCallbackCaller(*this, "myName"));
     * //someOtherObject will now call this->progress() from time to time, with the string "myName" as ID.
     * @endcode
     * 
     * @author Lena Brueder
     * @date 2012-08-13
     * @see ProgressCallback
     */
    class ProgressCallbackCaller
    {
    private:
        
    protected:
        std::string id;
        ProgressCallback& callback;
    public:
        /**
         * @brief Creates a new ProgressCallbackCaller object.
         * 
         * @param callback A reference to a callback object
         * @param id The id of the object. This is a name and may be freely chosen by the user. It will be used as id in ProgressCallbacks progress() function.
         */
        ProgressCallbackCaller(ProgressCallback& callback, const std::string& id = "") :
            id(id), callback(callback)
        {
            
        }
        /**
         * @brief Reports progress to an instance of ProgressCallback.
         * @see ProgressCallback
         * 
         * @param percent the amount of the job that is finished (0.0 <= percent <= 1.0), if applicable.
         *      if not applicable, percent < 0.0.
         * @param progressMessage an optional message that might be presented to the user.
         */
        virtual void progress(double percent, const std::string& progressMessage)
        {
            callback.progress(id, percent, progressMessage);
        }
    };
    
    class OutputStreamCallback : public ProgressCallbackCaller, private ProgressCallback
    {
    private:
        std::ostream& os;
    protected:
        
    public:
        OutputStreamCallback(std::ostream& os, const std::string& id = "") : ProgressCallbackCaller(*this, id), os(os) {}
        void progress(double percent, const std::string& progressMessage)
        {
            if (id != "")
                os << id << ": ";
            os << std::fixed << std::setprecision(2) << percent << "%, \"" << progressMessage << "\"" << std::endl;
        }
        void progress(const std::string& id, double percent, const std::string& progressMessage) {}
    };
}
#endif //PROGRESS_CALLBACK_HPP
